/**
 * project: Lab 4, part 1
 * @file part1.c
 * @brief 
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-27
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <AVRXlib/AVRX_Clocks.h>

#define CYCLE_LENGTH 5000
#define CCA_LENGTH 4800

#define SERIAL_BITS 0x03
#define SERIAL_STOP_BIT 0x00
#define SERIAL_PARITY 0x00

#define BSCALE_FACTOR -4
#define FBAUD 57600

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

static volatile short TIMER_STEP = 1;
volatile char *snd;

volatile short timerStarted = FALSE;
volatile uint16_t timerCount = 0;


/*Button push ISR*/
ISR(PORTJ_INT0_vect)
{
	if (!timerStarted)
	{
		if (RTC_CNT > 70)
		{
			timerStarted = TRUE;
			RTC_CNT = 0;
		}		
	}
	else
	{
		/*debounce start by waiting 20ms*/
		if (RTC_CNT > 70)
		{
			timerStarted = FALSE;
			timerCount = RTC_CNT;
			RTC_CNT = 0;
		}
	}
	//TIMER_STEP *= -1;
	//TCC0_CCA += TIMER_STEP;
	//PORTH_OUT = 0x00;
}

//ISR(TCC0_CCA_vect)
//{
	//PORTH_OUT = 0x00;	
//}
//
//
//ISR(TCC0_OVF_vect)
//{
	//if ( (TCC0_CCA > 0) && (TCC0_CCA < CCA_LENGTH) )
	//{
		//TCC0_CCA += TIMER_STEP;
	//}
	//PORTH_OUT = 0x01;
//}

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

	snd = dataString;
	USARTE1_DATA = *snd;
	//UsartWriteString("\n\r");
}			

/**
 * @brief 
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
	uint16_t nBSel;
	unsigned long sClk, pClk;
	cli();
	
/************ SET UP SYSTEM CLOCK *************/	
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
	
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
/************ SET UP TIMERS *************/
  
	/*Set source to the internal 1.024khz oscillator*/
	CLK_RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
	RTC_CTRL = RTC_PRESCALER_DIV1_gc;
	RTC_PER = 0xFFFF;
	RTC_CNT = 0;
	
	//TCC0_CTRLA = TC_CLKSEL_DIV8_gc; /*Set clock divider*/
	//TCC0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation **set to sigle slope**/
	//TCC0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	//TCC0_CTRLD = 0x00;  /*Turn off event action*/
	//TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	//TCC0_INTCTRLA = 0x02; /*Sets the interrupt to overflow to med priority*/
	//TCC0_INTCTRLB = 0x02; /*Sets the Compare or Capture interrupt to med priority*/
	//TCC0_PER = CYCLE_LENGTH; /* Cycle length */
	//TCC0_CCA = CCA_LENGTH; /* Capture and Compare point during cycle */
	
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
	
	/*Enable Tx*/
	USARTE1_CTRLB = 0x08;
	
	/*Set port direction to output*/
	PORTE.DIRSET = 0xFF;
	
/***************SET UP BUTTONS*******************/
	
	PORTJ_DIR = 0x00;
	PORTJ_INTCTRL = 0x01; /*sets interrupt 0 to med priority*/
	PORTJ_INT0MASK = 0xFF; /*sets all button pins to trigger interrupt*/
	PORTCFG_MPCMASK = 0xff;
	PORTJ_PIN0CTRL = 0x01;

	
	/*Set LED1 to output*/
	PORTH_DIR = 0xFF;
	PORTH_OUT = 0x01;
	
	sei();
	
	//PORTC_DIRSET = 0xFF;
	
	char buffer[15];

/************ PROGRAM LOOP *************/	
	while(1)
	{
		if (timerCount)
		{
			itoa(timerCount, buffer, 10);
			strcat(buffer, " ms\n\r");
			UsartWriteLine(buffer);
			timerCount = 0;
		}
	}
}
