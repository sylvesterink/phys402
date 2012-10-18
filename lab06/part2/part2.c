/**
 * project: Lab 6, part 2
 * @file part2.c
 * @brief 
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-16
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
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
#define PWM_CCA 4000
#define THROTTLE_MIN 5
/*#define PWM_STOP 500*/
#define THROTTLE_MAX 70

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

/************ SET UP TIMERS *************/
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc; /*Set clock divider*/
	TCC0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation to single slope*/
	TCC0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCC0_CTRLD = 0x00;  /*Turn off event action*/
	TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCC0_INTCTRLA = 0x00; /*Sets the interrupt to overflow to med priority*/
	TCC0_INTCTRLB = 0x00; /*Sets the Compare or Capture interrupt to med priority*/
	TCC0_PER = PWM_LENGTH; /* Cycle length */
	TCC0_CCA = PWM_CCA; /* Capture and Compare point during cycle */

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
	PORTC_DIR = 0xFF; /*Sets all the pins on PortC to output*/
    PORTC_OUT = 0x00; /*Pin 1 is pwm, pin 5 is direction*/
    
	sei(); /*Enable interrupts*/

/************ PROGRAM LOOP *************/
    int throttle;
    char msg[50];
    char rxBuf[RX_BUFSIZE];
    /*char txBuf[TX_BUFSIZE];*/

    /*Send initial message, then wait for Tx to complete*/
    USART_send(&stU, "Enter Throttle Value (5%-70%)");
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
                throttle = atoi(rxBuf);
                sprintf(msg, "Set new throttle %d%%", throttle);
                if (throttle < 0)
                {
                    throttle *= -1;
                    PORTC_OUT |= 0x10;
                }
                else
                {
                    PORTC_OUT &= 0xEF;
                }

                if ( (throttle >= THROTTLE_MIN) && (throttle <= THROTTLE_MAX) )
                {
                    TCC0_CCA = throttle * 100;

                    USART_send(&stU, msg);
                    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
                }
            }
        }
	}
}
