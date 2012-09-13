/**
 * project: Lab 2, part 1
 * @file part1.c
 * @brief Set a timer to implement a binary counter using the LEDs at the
 *        specified time rate
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-13
 * build: 1.0
 */
#include <avr/io.h>
#include <avr/interrupt.h>

/*With clock divider at 0x03, a full timer interval is 1 second*/
/*The clock delay is set here*/
#define MILLISECONDS 25
#define CLOCK_OFFSET (1000 - MILLISECONDS) * 65.535

/**
 * @brief Timer interrupt for when the timeout occurs
 * @param TIMER1_OVF_vect Type of interupt
 */
ISR(TIMER1_OVF_vect)
{
    /*Increment the binary counter*/
    PORTE += 0x01;

    cli();

    /*Start timer at a later point*/
    /*To get exact timer granularity, uncomment below*/
    /* At 0x03 clock scale, this will be 16us */
    /*TCNT1 = 65535; */

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
    /*Set the high bits of port E (LED bits) as output bits*/
    DDRE = 0xF0;
    /*Set all the LEDs off initially*/
    PORTE = 0x00;

    /*Disable other timer modes*/
    TCCR1A = 0x00;
    TCCR1C = 0x00;

    /*Clock divider*/
    /*Divide clock*/
    TCCR1B = 0x03;

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
