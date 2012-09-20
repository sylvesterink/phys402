/**
 * project: Lab 2, part 1
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

#define SERIAL_BITS 0x03
#define SERIAL_STOP_BIT 0x00
#define SERIAL_PARITY 0x00

#define BSCALE_FACTOR -4
#define FBAUD 57600

/*With clock divider at 0x03, a full timer interval is 1 second*/
#define MILLISECONDS 250
#define CLOCK_OFFSET (1000 - MILLISECONDS) * 65.535
#define CLK_DIVIDE 0x03

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Global value to indicate counter timeout*/
volatile short int timeout = FALSE;

/**
 * @brief !!
 * @param TIMER1_OVF_vect
 */
ISR(TCC0_OVF_vect)
{
    
    timeout = TRUE;
    /*This will allow us to set our own interval*/
    TCC0_CNT = CLOCK_OFFSET;
}

/**
 * @brief !!
 * @param USARTC1_TXC_vect
 */
ISR(USARTC1_TXC_vect)
{


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

/************ SET UP SERIAL PORT *************/	
	int nBScale = BSCALE_FACTOR;
	nBSel = (uint16_t)( (1.0 / pow(2.0,(double)nBScale)) * (double)((double)pClk / (16.0 * (double)FBAUD)) - 1.0);
	
	/*Set up serial port on port C */
	USARTC0_CTRLC = SERIAL_BITS | 
					SERIAL_STOP_BIT |
					SERIAL_PARITY;
					
	USARTC0_BAUDCTRLA = (unsigned char)(nBSel & 0x00FF);
	USARTC0_BAUDCTRLB = (char)( ((nBScale & 0x000F) << 4) |
						((nBSel & 0x0F00) >> 8) );
	/*set Tx interrupt to low priority*/						
	USARTC0_CTRLA = 0x0B;
	
	sei();
	
	/*Enable Tx*/
	USARTC0_CTRLB = 0x08;
	
	
	/*Set port direction to output*/
	PORTC.DIRSET = 0xFF;
	//PORTC_DIR = 0xFF;
	//PORTC.DIR |= (1<<3) | (1<<0);
	//PORTC.OUT |= (1<<3);	
/************ PROGRAM LOOP *************/	
	while(1)
	{
		if(/*timeout == */TRUE)
		{
			USARTC0_DATA = 'b';
			if(!(USARTC0.STATUS & USART_DREIF_bm))
			{
				while (!(USARTC0.STATUS & USART_TXCIF_bm));
			}
			USARTC1.STATUS |= USART_TXCIF_bm;
			
			timeout = FALSE;
		}
	}
	
}
