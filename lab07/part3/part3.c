/**
 * project: Lab 7, part 3
 * @file part3.c
 * @brief Cycles through states depending on serial inputs.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-16
 * build: 1.0
 * @todo *BUG* Delay between states.  This can be refined, but out of time.
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

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Define state function*/
void (*currentState)(int);

/*Usart instance*/
volatile XUSARTst stU;

/*define states*/
void searching();
void following();
void stopped();
void sendSignal();

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
	TCF0_INTCTRLA = 0x00; /*Sets the interrupt to overflow to med priority*/
	TCF0_INTCTRLB = 0x00; /*Sets the Compare or Capture interrupt to med priority*/
	TCF0_PER = CYCLE_LENGTH; /* Cycle length */
	TCF0_CCA = CCA_LENGTH; /* Capture and Compare point during cycle */

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
    currentState = &searching;
    char rxBuf[RX_BUFSIZE];
    int input = 0;

    /*Send initial message, then wait for Tx to complete*/
    USART_send(&stU, "Initialized");
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
                /*process input and send it to state*/
                input = atoi(rxBuf);
                (*currentState)(input);
            }
        }
	}
}

/**
 * @brief Searching state
 * @param input
 */
void searching(int input)
{
    /*newline to separate sample cycles*/
    USART_send(&stU, "Searching");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

    if (input == 1)
    {
        currentState = &following;
    }
}

/**
 * @brief Following state
 * @param input
 */
void following(int input)
{
    /*newline to separate sample cycles*/
    USART_send(&stU, "Follow");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

    if (input == 1)
    {
        currentState = &sendSignal;
    }
    else if (input == 2)
    {
        currentState = &searching;
    }
}

/**
 * @brief Stop state
 * @param input
 */
void stopped(int input)
{
    /*newline to separate sample cycles*/
    USART_send(&stU, "Stopped");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

    if (input == 1)
    {
        currentState = &searching;
    }
}

/**
 * @brief Send Signal state
 * @param input
 */
void sendSignal(int input)
{
    /*newline to separate sample cycles*/
    USART_send(&stU, "Signal");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

    if (input == 1)
    {
        currentState = &stopped;
    }
}

