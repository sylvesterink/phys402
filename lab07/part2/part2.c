/**
 * project: Lab 7, part 2
 * @file part2.c
 * @brief Unit samples input from ADC Pin0 for a sample period and records
 *        the average value.  After a specified number of average samples,
 *        it sends a list of those values to the serial port.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-16
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <AVRXlib/AVRX_Clocks.h>
#include <AVRXlib/AVRX_Serial.h>

/*Defines for setting up serial link*/
#define BSCALE_FACTOR -4
#define FBAUD 576

#define RX_BUFSIZE 160
#define TX_BUFSIZE 80

/*Define constants*/
#define OFF_TIME 6000
#define CYCLE_LENGTH 5000
#define CCA_LENGTH 4800
#define NUM_SAMPLES 100

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

volatile short sampleEnable = FALSE;

/*Usart instance*/
volatile XUSARTst stU;

/**
 * @brief ISR called whenever the high duration of the PWM ends.
 *        (This means the time matches the CCA value)
 *        Turn off the LED for the remainder of the cycle
 */
ISR(TCF0_CCA_vect)
{
    /*stop sampling*/
    sampleEnable = FALSE;
}


/**
 * @brief ISR called whenever the PWM cycle ends.
 *        (ie, when the counter overflows and restarts)
 *        This increases/decreases the duration of the CCA (for dimming)
 *        and turns on the LED for the new high duration of the cycle.
 */
ISR(TCF0_OVF_vect)
{
    /*start sampling*/
    sampleEnable = TRUE;
}

/**
 * @brief ISR called whenever a byte has been recieved via USART
 *        Calls the receive handler function of the serial library.
 */
ISR(USARTE1_RXC_vect)
{
    Rx_Handler(&stU);
}

/**
 * @brief ISR called whenever a byte has been sent via USART and it's ready
 *        for the next byte
 *        Calls the transmit handler function of the serial library.
 */
ISR(USARTE1_TXC_vect)
{
    Tx_Handler(&stU);
}

/**
 * @brief Set up serial port and ADC, then send the accelerometer value
 *        whenever a command is received on the serial line.
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
	unsigned long sClk, pClk;
	cli(); /*disable interrupts*/

/************ SET UP SYSTEM CLOCK *************/
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);

	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;

/************ SET UP TIMERS *************/

	TCF0_CTRLA = TC_CLKSEL_DIV64_gc; /*Set clock divider*/
	TCF0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation to single slope*/
	TCF0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCF0_CTRLD = 0x00;  /*Turn off event action*/
	TCF0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCF0_INTCTRLA = 0x02; /*Sets the interrupt to overflow to med priority*/
	TCF0_INTCTRLB = 0x02; /*Sets the Compare or Capture interrupt to med priority*/
	TCF0_PER = CYCLE_LENGTH; /* Cycle length */
	TCF0_CCA = CCA_LENGTH; /* Capture and Compare point during cycle */

/************ SET UP ADC *************/
    ADCB_CTRLA = 0x05; /*Enable, and use ADC Sample channel 1*/
    ADCB_CTRLB = ADC_RESOLUTION_12BIT_gc; /*Set the ADC conversion resolution*/
    ADCB_REFCTRL = ADC_REFSEL_AREFA_gc; /*Select the voltage reference
                                          (external ref on port A)*/
    ADCB_PRESCALER = ADC_PRESCALER_DIV512_gc; /*Prescale ADC clock input*/
    ADCB_CH0_CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc |
                    ADC_CH_GAIN_1X_gc; /*Set chan inputmode and gain*/
    ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN0_gc; /*Set pin to get value from*/

/************ SET UP SERIAL PORT *************/
    /*Initialize serial port to desired values*/
    USART_init(&stU,
            0xE1,
            pClk,
            (_USART_TXCIL_MED | _USART_RXCIL_MED),
            FBAUD,
            BSCALE_FACTOR,
            _USART_CHSZ_8BIT,
            _USART_PM_DISABLED,
            _USART_SM_1BIT);

    /*Initialize a buffer for incoming and outgoing serial transmissions*/
    USART_buffer_init(&stU,
                    RX_BUFSIZE,
                    TX_BUFSIZE);

    /*Set the input and output modes for the specified serial port.*/
    stU.fInMode = _INPUT_CR | _INPUT_ECHO;
	stU.fOutMode = _OUTPUT_CRLF;

    /*Enable specified serial port*/
    USART_enable(&stU, (USART_TXEN_bm | USART_RXEN_bm));

	sei(); /*Enable interrupts*/

/************ PROGRAM LOOP *************/
    long int adcBuf = 0, samples = 0;
    int sampleList[NUM_SAMPLES], i=0;
    char txBuf[TX_BUFSIZE];

    /*Send initial message, then wait for Tx to complete*/
    USART_send(&stU, "Initialized");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

	while(1)
	{
        /*If the sampler is enabled, accumulate samples from port*/
        if (sampleEnable)
        {
            /*Start ADC conversion*/
            ADCB.CH0.CTRL |= ADC_CH_START_bm;
            /*Wait for bit 1 to signal ADC conversion complete*/
            while (!ADCB_CH0_INTFLAGS) { ; }

            /*Accumulate sample and increment sample count*/
            adcBuf += ADCB_CH0_RES;
            samples++;
        }
        /*After sample cycle is complete, if we have a valid number of samples,
         * get the average and add it to a list, then prepare for next sample
         * cycle.*/
        else if ( (adcBuf > 0) && (samples > 0) )
        {
            /*Calculate sample average, add it to list*/
            adcBuf /= samples;
            sampleList[i] = adcBuf;

            /*increment list position, reset sample accumulator and counter*/
            i++;
            adcBuf = 0;
            samples = 0;
        }

        /*Once the list is full, output samples to serial port*/
        if (i > NUM_SAMPLES)
        {
            int j;
            /*Iterate through list, outputting each value*/
            for (j = 0; j < NUM_SAMPLES; ++j)
            {
                /*Convert to ascii*/
                itoa(sampleList[j], txBuf, 10);

                /*Display the value*/
                USART_send(&stU, txBuf);
                while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
            }

            /*newline to separate sample cycles*/
            USART_send(&stU, "\n\r");
            while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

            /*Reset list position*/
            i = 0;
        }
	}
}
