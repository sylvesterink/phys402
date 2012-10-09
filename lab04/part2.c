/**
 * project: Lab 4, part 2
 * @file part2.c
 * @brief Whenever a button is pushed, the LEDs toggle between bright and dim,
 *        slowly fading between states.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-09
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <AVRXlib/AVRX_Clocks.h> /*Clocks used for timing*/

/*Define constants*/
#define CYCLE_LENGTH 5000
#define CCA_LENGTH 4800

/*Define globals*/
/*This is the step size that determines how fast the CA length changes*/
static volatile short TIMER_STEP = 1;

/**
 * @brief ISR called whenever pin state changes.
 *        (ie, whenever a button is pushed)
 *        The timer step is flipped, changing whether the high duration of the
 *        PWM increases or decreases each cycle.
 */
ISR(PORTJ_INT0_vect)
{
	/*TIMER_STEP *= -1;*/ /*This works, but mult is slower*/
	TIMER_STEP -= TIMER_STEP; /*Flip value to its negative*/
	TCC0_CCA += TIMER_STEP; /*Add initial value to the CCA value*/
	PORTH_OUT = 0x00; /*Turn off LED for end of high duration*/
}

/**
 * @brief ISR called whenever the high duration of the PWM ends.
 *        (This means the time matches the CCA value)
 *        Turn off the LED for the remainder of the cycle
 */
ISR(TCC0_CCA_vect)
{
	PORTH_OUT = 0x00; /*Turn off LED*/
}


/**
 * @brief ISR called whenever the PWM cycle ends.
 *        (ie, when the counter overflows and restarts)
 *        This increases/decreases the duration of the CCA (for dimming)
 *        and turns on the LED for the new high duration of the cycle.
 */
ISR(TCC0_OVF_vect)
{
    /*If the value is between its high and low bounds, increase/decrease it
     * based on the timer step*/
	if ( (TCC0_CCA > 0) && (TCC0_CCA < CCA_LENGTH) )
	{
		TCC0_CCA += TIMER_STEP;
	}
	PORTH_OUT = 0xFF; /*Turn on LED*/
}

/**
 * @brief Set up timers, buttons etc and starts interrupt cycle.
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
	unsigned long sClk, pClk;
	cli(); /*Disable interrupts while setting up*/

/************ SET UP SYSTEM CLOCK *************/
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);

	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
/************ SET UP TIMERS *************/

	TCC0_CTRLA = TC_CLKSEL_DIV8_gc; /*Set clock divider*/
	TCC0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation to single slope*/
	TCC0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCC0_CTRLD = 0x00;  /*Turn off event action*/
	TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCC0_INTCTRLA = 0x02; /*Sets the interrupt to overflow to med priority*/
	TCC0_INTCTRLB = 0x02; /*Sets the Compare or Capture interrupt to med priority*/
	TCC0_PER = CYCLE_LENGTH; /* Cycle length */
	TCC0_CCA = CCA_LENGTH; /* Capture and Compare point during cycle */

/***************SET UP BUTTONS*******************/

	PORTJ_DIR = 0x00; /*Set this as an input pin*/
	PORTJ_INTCTRL = 0x01; /*sets interrupt 0 to med priority*/
	PORTJ_INT0MASK = 0xFF; /*sets all button pins to trigger interrupt*/
	PORTCFG_MPCMASK = 0xff; /*Set mask so that all pins are to be configured*/
	PORTJ_PIN0CTRL = 0x01; /*Set pin trigger on high edge, when button is pressed*/


/***************SET UP LEDS*******************/
	PORTH_DIR = 0xFF; /*Set LEDs to output*/
	PORTH_OUT = 0xFF; /*Turn LEDs on*/

	sei(); /*enable interrupts*/



/************ PROGRAM LOOP *************/
	while(1)
	{
	}
}
