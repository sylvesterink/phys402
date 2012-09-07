/**
 * project: Lab 2, part 2
 * @file part2.c
 * @brief 
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-
 * build: 1.0
 */
#include <avr/io.h>
#include <avr/interrupt.h>

/*With clock divider at 0x03, a full timer interval is 1 second*/
#define MILLISECONDS 25
#define CLOCK_OFFSET (1000 - MILLISECONDS) * 65.535

/*volatile int */
ISR(TIMER1_OVF_vect)
{
    /*PORTE += 0x10;*/
    if (PORTE == 0x00)
    {
        PORTE = 0x01;
    }
    else
    {
        PORTE = 0x00;
    }

    cli();

    /*Start timer at a later point*/
    /*To get exact timer granularity, uncomment below*/
    /* At 0x03 clock scale, this will be 16us */
    /*TCNT1 = 65535; */

    /*This will allow us to set our own interval*/
    TCNT1 = 65535;
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
    /*Set the high bits of port E (LED bits) as output bits*/
    /*DDRE = 0xF0;*/
    DDRE = 0x01;
    /*Set all the LEDs off initially*/
    PORTE = 0x00;

    /*Disable other timer modes*/
    TCCR1A = 0x00;
    TCCR1C = 0x00;

    /*Clock divider*/
    /*Divide clock*/
    TCCR1B = 0x01;

    /*Enable timer overflow interrupt*/
    TIMSK = 0x04;

    /*Clear timer overflow flag register*/
    TIFR = 0x00;

    sei();

    /* Run continuously */
    while (1) {
    }

    return 0;
}
