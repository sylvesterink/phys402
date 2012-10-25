/**
 * project: Lab 7, part 1
 * @file part1.c
 * @brief Times how long button was held, then changes led brightness based
 *        on how long button was held.  Longer is brighter, shorter is dimmer.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-10-09
 * build: 1.0
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

/*Define globals*/
/*This is the step size that determines how fast the CA length changes*/
static volatile short TIMER_STEP = 1;
volatile short timerStarted = FALSE;
volatile uint16_t timerCount = 0;

/*Usart instance*/
volatile XUSARTst stU;

/**
 * @brief ISR called whenever pin state changes.
 *        (ie, whenever a button is pushed)
 *        The timer step is flipped, changing whether the high duration of the
 *        PWM increases or decreases each cycle.
 */
ISR(PORTJ_INT0_vect)
{
	if (!timerStarted)
	{
        /*Only respond after 70ms to avoid button bounce*/
		if (RTC_CNT > 70)
		{
            USART_send(&stU, "Started timer");
            while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }

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
            if (timerCount > OFF_TIME)
            {
                timerCount = 0;
            }
            else if (timerCount > CYCLE_LENGTH)
			{
				timerCount = CYCLE_LENGTH;
			}
			TCF0_CCA = timerCount; /*sets the compare value to the button press length*/

            USART_send(&stU, "Stopped Timer");
            while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
		}
	}
    //TIMER_STEP *= -1; /*This works, but mult is slower*/
	///*TIMER_STEP -= TIMER_STEP; [>Flip value to its negative<]*/
	//TCF0_CCA += TIMER_STEP; /*Add initial value to the CCA value*/
	//PORTH_OUT = 0x00; /*Turn off LED for end of high duration*/
	
}

/**
 * @brief ISR called whenever the high duration of the PWM ends.
 *        (This means the time matches the CCA value)
 *        Turn off the LED for the remainder of the cycle
 */
ISR(TCF0_CCA_vect)
{
	PORTH_OUT = 0x00; /*Turn off LED*/
}


/**
 * @brief ISR called whenever the PWM cycle ends.
 *        (ie, when the counter overflows and restarts)
 *        This increases/decreases the duration of the CCA (for dimming)
 *        and turns on the LED for the new high duration of the cycle.
 */
ISR(TCF0_OVF_vect)
{
	PORTH_OUT = 0xFF; /*Turn on LED*/
}

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

	TCF0_CTRLA = TC_CLKSEL_DIV8_gc; /*Set clock divider*/
	TCF0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;/*Set waveform generation to single slope*/
	TCF0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
	TCF0_CTRLD = 0x00;  /*Turn off event action*/
	TCF0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
	TCF0_INTCTRLA = 0x02; /*Sets the interrupt to overflow to med priority*/
	TCF0_INTCTRLB = 0x02; /*Sets the Compare or Capture interrupt to med priority*/
	TCF0_PER = CYCLE_LENGTH; /* Cycle length */
	TCF0_CCA = CCA_LENGTH; /* Capture and Compare point during cycle */

/************ SET UP RTC Clock *************/
	/*Set source to the internal 1.024khz oscillator*/
	CLK_RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
	RTC_CTRL = RTC_PRESCALER_DIV1_gc; /*Set to actual value for ms timing*/
	RTC_PER = 0xFFFF; /*Set it to count full count value*/
	RTC_CNT = 0; /*Init counter to 0*/

/***************SET UP BUTTONS*******************/

	PORTJ_DIR = 0x00; /*Set this as an input pin*/
	PORTJ_INTCTRL = 0x01; /*sets interrupt 0 to med priority*/
	PORTJ_INT0MASK = 0xFF; /*sets all button pins to trigger interrupt*/
	PORTCFG_MPCMASK = 0xFF; /*Set mask so that all pins are to be configured*/
	PORTJ_PIN0CTRL = 0x00; /*Set pin trigger on both high/low edge*/

/************ SET UP SERIAL PORT *************/
    /*Initialize serial port to desired values*/
    USART_init(&stU,
            0xE1,  /* Will use port E1 */
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


/***************SET UP LEDS*******************/
	PORTH_DIR = 0xFF; /*Set LEDs to output*/
	PORTH_OUT = 0xFF; /*Turn LEDs on*/

	sei(); /*enable interrupts*/



/************ PROGRAM LOOP *************/
    /*Send initial message, then wait for Tx to complete*/
    USART_send(&stU, "Initialized");
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
    
	while(1)
	{
	}
}
