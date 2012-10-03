/**
 * project: Lab 3, part 2
 * @file part2.c
 * @brief Outputs a rotating text count from 1 to 5 on the USART E1 serial port
 *        at a rate of about 1 count per second.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-27
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <AVRXlib/AVRX_Clocks.h>
#include <math.h>

/* Define serial port settings*/
#define SERIAL_BITS 0x03
#define SERIAL_STOP_BIT 0x00
#define SERIAL_PARITY 0x00

#define BSCALE_FACTOR -4
#define FBAUD 57600

/*With clock divider at 0x07, 10,000 delay is 1 second*/
#define DELAY 10000
#define CLK_DIVIDE 0x07

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Global value to indicate counter timeout*/
volatile short int timeout = FALSE;

/*Buffer for sending a character to the serial port*/
volatile char *snd;

/**
 * @brief Interrupt handler for the timer timeout.  Sets a timeout flag to be
 *        used by the main loop.
 */
ISR(TCC0_OVF_vect)
{
    timeout = TRUE;
}

/**
 * @brief Interrupt handler for when a character is sent on the serial port.
 *        Sends the next character in a sequence (if there is one)
 * @param USARTC1_TXC_vect
 */
ISR(USARTE1_TXC_vect)
{
	if (*(++snd) != 0)
		USARTE1_DATA = *snd;
}

/**
 * @brief Outputs a string of text to the serial port E1
 * @param dataString The data to output
 */
void UsartWriteLine(char* dataString)
{
    /*Set the global buffer to point to the first character*/
	snd = dataString;
	USARTE1_DATA = *snd;
}			
 
/**
 * @brief Sets up timer and serial port, then starts sending a count from 1-5
 *        on the serial port at a one second interval.
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
	unsigned long sClk, pClk;
	uint16_t nBSel;
	
	cli();
/************ SET UP SERIAL CLOCK *************/	
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
	
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	
/************ SET UP TIMERS *************/
  
    TCC0_CTRLA = CLK_DIVIDE; /*Set clock divider*/
	TCC0_CTRLB = 0x00; /*Set waveform generation **set to normal**/
	TCC0_CTRLC = 0x00; /*Turn off waveform generation output compare off*/
	TCC0_CTRLD = 0x00; /*Turn off event action*/
	TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCC0_INTCTRLA = 0x02; /*Sets the interrupt to overflow, medium priority*/
	TCC0_INTCTRLB = 0x00; /*Sets the timer mode to compare*/
	TCC0_INTFLAGS = 0x00; /*Overflow flag in bit 0*/
	TCC0_PER = DELAY;

/************ SET UP SERIAL PORT *************/	
	int nBScale = BSCALE_FACTOR;
    /* Calculate baud rate */
	nBSel = (uint16_t)( (1.0 / pow(2.0,(double)nBScale)) * (double)((double)pClk / (16.0 * (double)FBAUD)) - 1.0);
	
	/*Set up serial port on port E1 */
	USARTE1_CTRLC = SERIAL_BITS | 
					SERIAL_STOP_BIT |
					SERIAL_PARITY;
					
	USARTE1_BAUDCTRLA = (unsigned char)(nBSel & 0x00FF);
	USARTE1_BAUDCTRLB = (char)( ((nBScale & 0x000F) << 4) |
						((nBSel & 0x0F00) >> 8) );

	/*set Tx interrupt to medium priority*/						
	USARTE1_CTRLA = 0x08;
	
	sei();
	
	/*Enable Tx on E1*/
	USARTE1_CTRLB = 0x08;
	
	
	/*Set port direction to output*/
	PORTE.DIRSET = 0xFF;

/************ PROGRAM LOOP *************/	
    /* Set up output strings */
	char* numbers[5] = {"one\n\r", "two\n\r", "three\n\r", "four\n\r", "five\n\r"};
	int numberCount = 0;

    /* Wait for the appropriate timeout, then write the specified line to the
     * serial port*/
	while(1)
	{
		if(timeout == TRUE)
		{
			timeout = FALSE;
			UsartWriteLine(numbers[numberCount]);
            /* Cycle through the index value of 0-4 repeatedly */
            numberCount++;
			numberCount %= 5;
			/*numberCount = (++numberCount)%5;*/
		}
	}
}
