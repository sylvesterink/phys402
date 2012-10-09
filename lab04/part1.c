/**
 * project: Lab 4, part 1
 * @file part1.c
 * @brief Generate a pulse waveform on an output pin for measurement
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-09
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>

/**
 * @brief Sets up the timers to generate a PWM with an 8% duty cycle
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
	
/************ SET UP TIMERS *************/
  
	TCC0_CTRLA = TC_CLKSEL_DIV8_gc; /*Set clock divider*/

    /*Set waveform generation to single slope**/
	TCC0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;
	TCC0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCC0_CTRLD = 0x00;  /*Turn off event action*/
	TCC0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCC0_INTCTRLA = 0x00; /*Sets the interrupt to overflow off*/
	TCC0_INTCTRLB = 0x00; /*Sets the Compare or Capture interrupt off*/
	TCC0_PER = 2000; /* Cycle length */
	TCC0_CCA = 160; /* Capture and Compare point during cycle */
	
	PORTC_DIRSET = 0xFF; /*Sets all the pins on PortC to be output pins*/

/************ PROGRAM LOOP *************/	
	while(1)
	{
	}
}
