/**
 * project: Lab 3, part 1
 * @file part1.c
 * @brief 
 *        
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-13
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <AVRXlib/AVRX_Clocks.h>
#include <math.h>
#include <stdio.h>
//#include <string.h>

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

volatile char *snd;

/**
 * @brief !!
 * @param TIMER1_OVF_vect
 */
ISR(TCC0_OVF_vect)
{
    
    timeout = TRUE;
    /*This will allow us to set our own interval*/
//    TCC0_CNT = CLOCK_OFFSET;
}

/**
 * @brief !!
 * @param USARTC1_TXC_vect
 */
ISR(USARTE1_TXC_vect)
{
	if (*(++snd) != 0)
//		UsartWriteChar(*dataString++);
		USARTE1_DATA = *snd;
}


void UsartWriteLine(char* dataString)
{
//	UsartWriteString(dataString);

	snd = dataString;
	USARTE1_DATA = *snd;
//	UsartWriteString("\n\r");
}			
 
void main(void)
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
	nBSel = (uint16_t)( (1.0 / pow(2.0,(double)nBScale)) * (double)((double)pClk / (16.0 * (double)FBAUD)) - 1.0);
	
	/*Set up serial port on port C */
	USARTE1_CTRLC = SERIAL_BITS | 
					SERIAL_STOP_BIT |
					SERIAL_PARITY;
					
	USARTE1_BAUDCTRLA = (unsigned char)(nBSel & 0x00FF);
	USARTE1_BAUDCTRLB = (char)( ((nBScale & 0x000F) << 4) |
						((nBSel & 0x0F00) >> 8) );
	/*set Tx interrupt to low priority*/						
	USARTE1_CTRLA = 0x08;
	
	sei();
	
	/*Enable Tx*/
	USARTE1_CTRLB = 0x08;
	
	
	/*Set port direction to output*/
	PORTE.DIRSET = 0xFF;

/************ PROGRAM LOOP *************/	
	char* numbers[5] = {"one\n\r", "two\n\r", "three\n\r", "four\n\r", "five\n\r"};
	int numberCount = 0;
	while(1)
	{
		if(timeout == TRUE)
		{
			timeout = FALSE;
			UsartWriteLine(numbers[numberCount]);
			numberCount = (++numberCount)%5;
			
		}
	}
	
}
