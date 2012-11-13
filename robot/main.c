/**
 * project: robot
 * @file main.c
 * @brief Main implementation of the Snoop Dogg robot.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-12
 * build: 1.0
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <AVRXlib/AVRX_Clocks.h>
#include <AVRXlib/AVRX_Serial.h>

/*Defines for setting up serial link*/
#define BSCALE_FACTOR -4
#define FBAUD 576

#define RX_BUFSIZE 160
#define TX_BUFSIZE 80

/*Define constants*/
/*Default pwm length*/
#define PWM_LENGTH 10000

/*Default wheel speed*/
#define PWM_WHEEL_DEFAULT 9500

/*Set max/min values for eye servos*/
#define PWM_SERVO_MIN 500
#define PWM_SERVO_DEFAULT 750
/*#define PWM_STOP 500*/
#define PWM_SERVO_MAX 1000

/*Min/max percentages for dc motors*/
#define THROTTLE_MIN 20 /*5*/
#define THROTTLE_MAX 99

/*#define LEFT 0x01*/
/*#define RIGHT 0x02*/

#define LFORWARD 0x80
#define RFORWARD 0x20
#define LREVERSE 0x40
#define RREVERSE 0x10

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Usart instance*/
volatile XUSARTst stU;

/*Function definitions*/
void serPrint(char* buf);
uint8_t serRead(char* buf);
void move(uint8_t wheelSel, uint8_t lThrottle, uint8_t rThrottle);
void actuateEyes(int lEyePos, int rEyePos);

/**
 * @brief ISR called whenever a byte has been recieved via USART
 *        Calls the receive handler function of the serial library.
 */
ISR(USARTC0_RXC_vect)
{
    Rx_Handler(&stU);
}

/**
 * @brief ISR called whenever a byte has been sent via USART and it's ready
 *        for the next byte
 *        Calls the transmit handler function of the serial library.
 */
ISR(USARTC0_TXC_vect)
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
    /* set up wheel motors */
    TCE1_CTRLA = TC_CLKSEL_DIV64_gc; /*Set clock divider*/
    TCE1_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm | TC0_CCBEN_bm;/*Set waveform generation to single slope, enable first two pins for pwm*/
    TCE1_CTRLC = 0x00; /*Turn waveform generation output compare off*/
    TCE1_CTRLD = 0x00;  /*Turn off event action*/
    TCE1_CTRLE = 0x00; /*Sets timer to 16bit mode*/
    TCE1_INTCTRLA = 0x00; /*Sets the interrupt to overflow to med priority*/
    TCE1_INTCTRLB = 0x00; /*Sets the Compare or Capture interrupt to med priority*/
    TCE1_PER = PWM_LENGTH; /* Cycle length */
    TCE1_CCA = PWM_WHEEL_DEFAULT; /* Capture and Compare point during cycle */
    TCE1_CCB = PWM_WHEEL_DEFAULT; /* Capture and Compare point during cycle */

    /* set up eye servos */
    TCE0_CTRLA = TC_CLKSEL_DIV64_gc; /*Set clock divider*/
    TCE0_CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm | TC0_CCBEN_bm;/*Set waveform generation to single slope, enable first two pins for pwm*/
    TCE0_CTRLC = 0x00; /*Turn waveform generation output compare off*/
    TCE0_CTRLD = 0x00;  /*Turn off event action*/
    TCE0_CTRLE = 0x00; /*Sets timer to 16bit mode*/
    TCE0_INTCTRLA = 0x00; /*Sets the interrupt to overflow to med priority*/
    TCE0_INTCTRLB = 0x00; /*Sets the Compare or Capture interrupt to med priority*/
    TCE0_PER = PWM_LENGTH; /* Cycle length */
    TCE0_CCA = PWM_SERVO_DEFAULT; /* Capture and Compare point during cycle */
    TCE0_CCB = PWM_SERVO_DEFAULT; /* Capture and Compare point during cycle */

/************ SET UP RTC Clock *************/
	/*Set source to the internal 1.024khz oscillator*/
    //CLK_RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
    //RTC_CTRL = RTC_PRESCALER_DIV1_gc; /*Set to actual value for ms timing*/
    //RTC_PER = 0xFFFF; /*Set it to count full count value*/
    //RTC_CNT = 0; /*Init counter to 0*/

/***************SET UP BUTTONS*******************/
	/*PORTJ_DIR = 0x00; [>Set this as an input pin<]*/
	/*PORTJ_INTCTRL = 0x01; [>sets interrupt 0 to med priority<]*/
	/*PORTJ_INT0MASK = 0xFF; [>sets all button pins to trigger interrupt<]*/
	/*PORTCFG_MPCMASK = 0xFF; [>Set mask so that all pins are to be configured<]*/
	/*PORTJ_PIN0CTRL = 0x00; [>Set pin trigger on both high/low edge<]*/

/************ SET UP SERIAL PORT *************/
    /*Initialize serial port to desired values*/
    USART_init(&stU,
            0xC0,  /* Will use port C0 */
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
    /*TODO remove*/
    //PORTH_DIR = 0xFF; /*Set LEDs to output*/
    //PORTH_OUT = 0xFF; /*Turn LEDs on*/

	sei(); /*enable interrupts*/

    /* Enable output to DC motors*/
	PORTK_DIR = 0xFF; /*Sets all the pins on PortK to output*/
    PORTK_OUT = 0x00;

    /* Enable output to servos (lo) and H-bridge (hi)*/
	PORTE_DIR = 0xFF; /*Sets all the pins on PortE to output*/
    PORTE_OUT = 0x00;

    /* Enable input from color sensors*/
    //PORTD_DIR = 0x00; /* Set all pins on PortD to input*/
    //PORTD_DIR = 0x00; /* Set all pins on PortD to input*/

    /*set up remaining serial settings*/
	PORTC_DIR |= 0X03;
	PORTC_OUT &= 0XFE; /*turn off PC0 to enable transceiver*/
	PORTC_OUT |= 0X02; /*turn on PC1 to take transceiver out of shutdown mode*/

/************ PROGRAM LOOP *************/
    /*Send initial message, then wait for Tx to complete*/
    serPrint("Initialized");
    
    /*char msg[50]; [> buffer for confirmation message <]*/
    char rxBuf[RX_BUFSIZE];

    /*DC Wheel values*/
    uint8_t throttle;
    uint8_t wheels = 0;

    /*Servo values*/
    int servoValue = 0;

    /*sensor values*/
    int test = 0;

	while(1)
	{
        if (serRead(rxBuf) == TRUE)
        {
            /* INCORRECT test color sensors
            test = PORTD_IN;
            char msg[50];
            itoa(test, msg, 10);
            serPrint(msg);
            */

            /*  *** ACTUATE EYES ***
            servoValue = atoi(rxBuf);
            actuateEyes(servoValue, servoValue);
            */

            /*  *** MOVE WHEELS ***
            switch (rxBuf[0])
            {
                case 'L':
                    wheels = LFORWARD;
                    break;
                case 'l':
                    wheels = LREVERSE;
                    break;
                case 'R':
                    wheels = RFORWARD;
                    break;
                case 'r':
                    wheels = RREVERSE;
                    break;
                case 'B':
                    wheels = LFORWARD | RFORWARD;
                    break;
                case 'b':
                    wheels = LFORWARD | RREVERSE;
                    break;
                case 's':
                    wheels = 0x00;
                    break;
                default:
                    wheels = 0x00;
                    break;
            }
            */

            /* convert throttle value */
            /*throttle = atoi(&rxBuf[1]);*/

            /*move(wheels, throttle, throttle);*/
        }
	}
}

void serPrint(char* buf)
{
    USART_send(&stU, buf);
    while (!(stU.serStatus & _USART_TX_EMPTY) ) { ; }
}

uint8_t serRead(char* buf)
{
    /*Wait until input termination arrives*/
    if (stU.serStatus & _USART_RX_DONE)
    {
        /*Buffer string locally and clear Rx register*/
        USART_read(&stU, buf);

        /*If it's a valid command, process it*/
        if (strlen(buf) > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void move(uint8_t wheelSel, uint8_t lThrottle, uint8_t rThrottle)
{
    /*TODO: Need to validate wheelSel value?
     *      Are checks needed?*/
    PORTK_OUT = wheelSel;

    /* Ensure that the throttle value is within allowed range */
    /*then set pwm based on throttle percentages*/
    if ( (lThrottle >= THROTTLE_MIN) && (lThrottle <= THROTTLE_MAX) )
    {
        TCE1_CCA = lThrottle * 100;
    }

    if ( (rThrottle >= THROTTLE_MIN) && (rThrottle <= THROTTLE_MAX) )
    {
        TCE1_CCB = rThrottle * 100;
    }
}

void actuateEyes(int lEyePos, int rEyePos)
{
    if (lEyePos >= PWM_SERVO_MIN && lEyePos <= PWM_SERVO_MAX)
        TCE0_CCA = lEyePos;

    if (rEyePos >= PWM_SERVO_MIN && rEyePos <= PWM_SERVO_MAX)
        TCE0_CCB = rEyePos;
}
