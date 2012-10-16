/**
 * project: Lab 6, part 1
 * @file part1.c
 * @brief 
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

/*Define PWM default values*/
#define PWM_LENGTH 10000
#define PWM_MIN 500
/*#define PWM_STOP 500*/
#define PWM_MAX 1000

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Usart instance*/
volatile XUSARTst stU;

/**
 * @brief ISR called whenever the high duration of the PWM ends.
 *        (This means the time matches the CCA value)
 */
ISR(TCC0_CCA_vect)
{
    PORTC_OUT = 0x00;
}


/**
 * @brief ISR called whenever the PWM cycle ends.
 *        (ie, when the counter overflows and restarts)
 */
ISR(TCC0_OVF_vect)
{
    PORTC_OUT = 0xFF;
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
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc; /*Set clock divider*/
	TCC0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation to single slope*/
	TCC0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCC0_CTRLD = 0x00;  /*Turn off event action*/
	TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCC0_INTCTRLA = 0x02; /*Sets the interrupt to overflow to med priority*/
	TCC0_INTCTRLB = 0x02; /*Sets the Compare or Capture interrupt to med priority*/
	TCC0_PER = PWM_LENGTH; /* Cycle length */
	TCC0_CCA = PWM_MIN; /* Capture and Compare point during cycle */

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

/************ SET UP OUTPUT PORT *************/
	PORTC_DIR = 0xFF; /*Sets all the pins on PortC to be output pins*/

	sei(); /*Enable interrupts*/

/************ PROGRAM LOOP *************/
    int newCCA;
    char rxBuf[RX_BUFSIZE];
    /*char txBuf[TX_BUFSIZE];*/

    /*Send initial message, then wait for Tx to complete*/
    USART_send(&stU, "Enter PWM Value Between 500-1000");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

	while(1)
	{
        /*Wait until input termination arrives*/
        if (stU.serStatus & _USART_RX_DONE)
        {
            /*Buffer string locally and clear Rx register*/
            USART_read(&stU, rxBuf);

            /*If it's a valid command, process it*/
            if (strlen(rxBuf) > 0)
            {
                newCCA = atoi(rxBuf);
                if ( (newCCA >= PWM_MIN) && (newCCA <= PWM_MAX) )
                {
                    TCC0_CCA = newCCA;
                    USART_send(&stU, "Set new PWM");
                    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
                }
            }
        }
	}
}Enter PWM Value
