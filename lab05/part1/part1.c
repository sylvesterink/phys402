/**
 * project: Lab 5, part 1
 * @file part1.c
 * @brief Unit listens for a command on the serial port, then replies with the
 *        current value of one of the accelerometer axes.
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

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Usart instance*/
volatile XUSARTst stU;

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

/************ SET UP ADC *************/
    ADCA_CTRLA = 0x05; /*Enable, and use ADC Sample channel 1*/
    ADCA_CTRLB = ADC_RESOLUTION_12BIT_gc; /*Set the ADC conversion resolution*/
    ADCA_REFCTRL = ADC_REFSEL_AREFA_gc; /*Select the voltage reference
                                          (external ref on port A)*/
    ADCA_PRESCALER = ADC_PRESCALER_DIV512_gc; /*Prescale ADC clock input*/
    ADCA_CH0_CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc |
                    ADC_CH_GAIN_1X_gc; /*Set chan inputmode and gain*/
    ADCA_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN5_gc; /*Set pin to get value from*/

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
    int adcBuf;
    char rxBuf[RX_BUFSIZE];
    char txBuf[TX_BUFSIZE];

    /*Send initial message, then wait for Tx to complete*/
    USART_send(&stU, "What is your bidding my master?");
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
                /*Start ADC conversion*/
                ADCA.CH0.CTRL |= ADC_CH_START_bm;
                /*Wait for bit 1 to signal ADC conversion complete*/
                while (!ADCA_CH0_INTFLAGS) { ; }

                /*Buffer ADCA value and convert to a string for display*/
                adcBuf = ADCA_CH0_RES;
                itoa(adcBuf, txBuf, 10);

                /*Display the value*/
                USART_send(&stU, txBuf);
                while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
            }
        }
	}
}
