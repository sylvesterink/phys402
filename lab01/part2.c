/**
 * project: Lab 1, parts 1 & 2
 * @file main.c
 * @brief Implements a simple binary counter using the LEDs on an atmega64
 *        uP board.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-04
 * build: 1.0
 */
#include <avr/io.h>

/**
 * @brief An estimated number of cycles to loop for a 1 ms delay
 * @warning This value isn't exact, due to compiler quirks.  Use only for
 *          learning purposes.
 */
#define CYCLES_PER_MS 100000

/*Function Declarations*/
void wait_ms(unsigned int time);

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

    /* Turn on each LED in a binary sequence, by adding to port E's value
     * Wait a set period (in ms) between each change*/
    while (1) {
        PORTE += 0x10;
        wait_ms(250);
    }

    return 0;
}

/**
 * @brief Delay for a set number of milliseconds through a spinlock.
 * @param time The number of milliseconds to delay.
 * @warning The actual delay time isn't exact, due to compiler quirks.
 */
void wait_ms(unsigned int time)
{
    /*volatile is used in order to slow down the loop cycle*/
    volatile unsigned int cycles = time * CYCLES_PER_MS;

    /*spinlock delay*/
    while (cycles > 0) {
        cycles--;
    }
}

