/**
 * project: Lab 5, part 2
 * @file part2.c
 * @brief
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-09
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

const int muxPin[] = {ADC_CH_MUXPOS_PIN5_gc,
                        ADC_CH_MUXPOS_PIN6_gc,
                        ADC_CH_MUXPOS_PIN7_gc};

/*Usart instance*/
volatile XUSARTst stU;

/*Character send buffer*/
volatile char *snd;

/*Function declarations*/
void UsartWriteLine(char* dataString);

/**
 * @brief ISR called whenever a byte has been recieved via USART
 */
ISR(USARTE1_RXC_vect)
{
    Rx_Handler(&stU);
}

/**
 * @brief ISR called whenever a byte has been sent via USART and it's ready
 *        for the next byte
 *        If there's a value in the send buffer, write it to the USART to send.
 */
ISR(USARTE1_TXC_vect)
{
    Tx_Handler(&stU);
}

/**
 * @brief Write string to USART
 * @param dataString String to be sent
 */
void UsartWriteLine(char* dataString)
{
	snd = dataString; /*Point the buffer to the first character*/
	USARTE1_DATA = *snd; /*Send the first buffered character*/
}

/**
 * @brief Set up buttons, timers, serial ports, then whenever the timer goes
 *        off, send the timer value out on the serial line.
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
    ADCA_REFCTRL = ADC_REFSEL_AREFA_gc; /*Select the voltage reference (external ref on port A*/
    ADCA_PRESCALER = ADC_PRESCALER_DIV32_gc; /*Prescale ADC clock input*/
    ADCA_CH0_CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc | /*Set chan inputmode and gain*/
                    ADC_CH_GAIN_1X_gc;
    ADCA_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN5_gc; /*Set pin to get value from*/

/************ SET UP SERIAL PORT *************/
    USART_init(&stU,
            0xE1,
            pClk,
            (_USART_TXCIL_MED | _USART_RXCIL_MED),
            FBAUD,
            BSCALE_FACTOR,
            _USART_CHSZ_8BIT,
            _USART_PM_DISABLED,
            _USART_SM_1BIT);

    USART_buffer_init(&stU,
                    RX_BUFSIZE,
                    TX_BUFSIZE);

    stU.fInMode = _INPUT_CR | _INPUT_ECHO;
	stU.fOutMode = _OUTPUT_CRLF;

    USART_enable(&stU, (USART_TXEN_bm | USART_RXEN_bm));

	sei(); /*Enable interrupts*/

/************ PROGRAM LOOP *************/
    int adcBuf[3];
    int i = 0;
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
                for (i = 0; i < 3; ++i)
                {
                    ADCA_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN5_gc; //muxPin[i]; /*Set pin to get value from*/
                    ADCA.CH0.CTRL |= ADC_CH_START_bm;
                    /*Wait for bit 1 to signal ADC conversion complete*/
                    while (!ADCA_CH0_INTFLAGS) { ; }

                    adcBuf[i] = ADCA_CH0_RES;
                    itoa(adcBuf[0], txBuf, 10);

                    USART_send(&stU, txBuf);
                    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
                }

                /*sprintf(txBuf, "%d %d %d\n", adcBuf[0], adcBuf[1], adcBuf[2]);*/
            }
        }
	}
}
