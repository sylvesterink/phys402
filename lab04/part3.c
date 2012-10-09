/**
 * project: Lab 4, part 3
 * @file part3.c
 * @brief Times the pause between button presses and outputs it on the serial
 *        link.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-09
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <AVRXlib/AVRX_Clocks.h>

/*Defines for setting up serial link*/
#define SERIAL_BITS 0x03
#define SERIAL_STOP_BIT 0x00
#define SERIAL_PARITY 0x00

#define BSCALE_FACTOR -4
#define FBAUD 57600

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Character send buffer*/
volatile char *snd;

/*Globals for whether the timer has started and the timer value to output*/
volatile short timerStarted = FALSE;
volatile uint16_t timerCount = 0;


/**
 * @brief ISR called whenever pin state changes.
 *        (ie, whenever a button is pushed)
 *        If the timer is running, stop it and get the timer count,
 *        otherwise enable the timer and reset the count to 0
 */
ISR(PORTJ_INT0_vect)
{
	if (!timerStarted)
	{
        /*Only respond after 70ms to avoid button bounce*/
		if (RTC_CNT > 70)
		{
			timerStarted = TRUE;
			RTC_CNT = 0; /*Set the timer counter to 0*/
		}
	}
	else
	{
        /*Only respond after 70ms to avoid button bounce*/
		if (RTC_CNT > 70)
		{
			timerStarted = FALSE;
			timerCount = RTC_CNT; /*Get the timer count and store it for printing*/
			RTC_CNT = 0; /*Timer counter set to 0 for avoiding button bounce*/
		}
	}
}

/**
 * @brief ISR called whenever a byte has been sent via USART and it's ready
 *        for the next byte
 *        If there's a value in the send buffer, write it to the USART to send.
 */
ISR(USARTE1_TXC_vect)
{
	if (*(++snd) != 0)
		USARTE1_DATA = *snd;
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
	uint16_t nBSel;
	unsigned long sClk, pClk;
	cli(); /*disable interrupts*/

/************ SET UP SYSTEM CLOCK *************/
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);

	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;

/************ SET UP TIMERS *************/
	/*Set source to the internal 1.024khz oscillator*/
	CLK_RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
	RTC_CTRL = RTC_PRESCALER_DIV1_gc; /*Set to actual value for ms timing*/
	RTC_PER = 0xFFFF; /*Set it to count full count value*/
	RTC_CNT = 0; /*Init counter to 0*/

/************ SET UP SERIAL PORT *************/
	int nBScale = BSCALE_FACTOR;
	nBSel = (uint16_t)( (1.0 / pow(2.0,(double)nBScale)) * (double)((double)pClk / (16.0 * (double)FBAUD)) - 1.0);

	/*Set up serial port on port C */
	USARTE1_CTRLC = SERIAL_BITS |
					SERIAL_STOP_BIT |
					SERIAL_PARITY;

    /*Set baud controls*/
	USARTE1_BAUDCTRLA = (unsigned char)(nBSel & 0x00FF);
	USARTE1_BAUDCTRLB = (char)( ((nBScale & 0x000F) << 4) |
						((nBSel & 0x0F00) >> 8) );

	USARTE1_CTRLA = 0x08; /*set Tx interrupt to low priority*/
	USARTE1_CTRLB = 0x08; /*Enable Tx*/
	PORTE.DIRSET = 0xFF; /*Set port direction to output*/

/***************SET UP BUTTONS*******************/
	PORTJ_DIR = 0x00; /*Set this as an input pin*/
	PORTJ_INTCTRL = 0x01; /*sets interrupt 0 to med priority*/
	PORTJ_INT0MASK = 0xFF; /*sets all button pins to trigger interrupt*/
	PORTCFG_MPCMASK = 0xff; /*Set mask so that all pins are to be configured*/
	PORTJ_PIN0CTRL = 0x01; /*Set pin trigger on high edge, when button is pressed*/

/***************SET UP LEDS*******************/
	PORTH_DIR = 0xFF; /*Set LEDs to output*/
	PORTH_OUT = 0x01; /*Turn LED1 on*/

	sei(); /*Enable interrutps*/

/************ PROGRAM LOOP *************/
	char buffer[15]; /*Buffer to contain string to print to serial*/

	while(1)
	{
        /*If the timer count has been set, then we have a time to send*/
		if (timerCount)
		{
			itoa(timerCount, buffer, 10); /*Convert it to a char string to send*/
			strcat(buffer, " ms\n\r"); /*Add ms and newline to string*/
			UsartWriteLine(buffer); /*write it to USART*/
			timerCount = 0; /*Reinit the timer for next round*/
		}
	}
}
