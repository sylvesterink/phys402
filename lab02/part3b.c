/**
 * project: Lab 2, part 3
 * @file part2.c
 * @brief 
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-
 * build: 1.0
 */
#include <avr/io.h>
#include <avr/interrupt.h>

/*With clock divider at 0x03, a full timer interval is 1 second*/
#define MILLISECONDS 250
#define CLOCK_OFFSET (1000 - MILLISECONDS) * 65.535

#define FALSE 0
#define TRUE 1

volatile short int timeout = FALSE;

/*volatile int */
ISR(INT4_vect)
{
    if (PINC & 0x01)
    {
        timeout = TRUE;
    }

    /*This will allow us to set our own interval*/
    TCNT1 = CLOCK_OFFSET;
}

/**
 * @brief Main program loop.  Repeatedly counts from 0-15 using the onboard
 *        LEDs as indicators.
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
    timeout = FALSE;
    short int divider = 0;
    
    /*Set the high bits of port E (LED bits) as output bits*/
    DDRE = 0xF0;
    /*Set all the LEDs off initially*/
    PORTE = 0x00;

    /*Set the low bits of port D (pin bits) as output bits*/
    DDRD = 0x0F;
    /*Set all the pins off initially*/
    PORTD = 0x00;

    /*Set the low bits of port B (pin bits) as input bits*/
    DDRC = 0x00;

    /*Set to detect INT4 on rising edge (so we know when button pressed)*/
    EICRB = 0x02;

    /*!!! don't know what we're doing here*/
    EIMSK = 0x08;
    EIFR = 0x10;

    sei();

    /* Run continuously */
    while (1) {
        if (timeout == TRUE)
        {
            PORTE += 0x10;
            if (divider > 2)
            {
                PORTD += 0x01;
                divider = 0;
            }

            divider++;
            timeout = FALSE;
        }
    }

    return 0;
}
