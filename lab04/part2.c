/**
 * project: Lab 4, part 2
 * @file part2.c
 * @brief 
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-27
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <AVRXlib/AVRX_Clocks.h>

#define CYCLE_LENGTH 5000
#define CCA_LENGTH 4800

static volatile short TIMER_STEP = 1;

ISR(PORTJ_INT0_vect)
{
	TIMER_STEP *= -1;
	TCC0_CCA += TIMER_STEP;
	PORTH_OUT = 0x00;
}

ISR(TCC0_CCA_vect)
{
	PORTH_OUT = 0x00;	
}


ISR(TCC0_OVF_vect)
{
	if ( (TCC0_CCA > 0) && (TCC0_CCA < CCA_LENGTH) )
	{
		TCC0_CCA += TIMER_STEP;
	}
	PORTH_OUT = 0xFF;
}

/**
 * @brief 
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
	unsigned long sClk, pClk;
	cli();
	
/************ SET UP SYSTEM CLOCK *************/	
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
	
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
/************ SET UP TIMERS *************/
  
	TCC0_CTRLA = TC_CLKSEL_DIV8_gc; /*Set clock divider*/
	TCC0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation **set to sigle slope**/
	TCC0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCC0_CTRLD = 0x00;  /*Turn off event action*/
	TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCC0_INTCTRLA = 0x02; /*Sets the interrupt to overflow to med priority*/
	TCC0_INTCTRLB = 0x02; /*Sets the Compare or Capture interrupt to med priority*/
	TCC0_PER = CYCLE_LENGTH; /* Cycle length */
	TCC0_CCA = CCA_LENGTH; /* Capture and Compare point during cycle */
	
/***************SET UP BUTTONS*******************/
	
	PORTJ_DIR = 0x00;
	PORTJ_INTCTRL = 0x01; /*sets interrupt 0 to med priority*/
	PORTJ_INT0MASK = 0xFF; /*sets all button pins to trigger interrupt*/
	PORTCFG_MPCMASK = 0xff;
	PORTJ_PIN0CTRL = 0x01;

	
	/*Set LED1 to output*/
	PORTH_DIR = 0xFF;
	PORTH_OUT = 0xFF;
	
	sei();

	

/************ PROGRAM LOOP *************/	
	while(1)
	{
	}
}
