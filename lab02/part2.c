/**
 * project: Lab 2, part 2
 * @file part2.c
 * @brief Flips the value of an output pin at a fast timer rate in order
 *        to test the delay of that timer.  Can optionally count up on the
 *        binary counter, as in part 1
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-13
 * build: 1.0
 */
#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * @brief Whenever the timeout occurs, flip the output value on the specified
 *        port.
 * @param TIMER1_OVF_vect The interrupt to process
 * @note We can't really test properly with oscope if it's set to a binary
 *       counter so we'll just flip the pin.
 */
ISR(TIMER1_OVF_vect)
{
    /*Old binary counter method*/
    /*PORTE += 0x10;*/

    /*New pin flip method*/
    if (PORTE == 0x00)
    {
        PORTE = 0x01;
    }
    else
    {
        PORTE = 0x00;
    }

    cli();

    /*Set the starting point of the timer.  65535 is the max, meaning a
     * timeout will occur every tick of the timer*/
    TCNT1 = 65535;
}

/**
 * @brief Main program loop.  Repeatedly flips the output pin using a timer
 *        interrupt
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
    /*Set the first bit of port E as output bit*/
    DDRE = 0x01;
    /*Set all the output bits off initially*/
    PORTE = 0x00;

    /*Disable other timer modes*/
    TCCR1A = 0x00;
    TCCR1C = 0x00;

    /*Clock divider*/
    /*Don't divide clock*/
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
