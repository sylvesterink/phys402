/**
 * project: Lab 1, parts 1 & 2
 * @file main.c
 * @brief
 * @author Cameron Bently, Brandon Kasa
 * @date 2012-08-30
 * build:
 */
#include <avr/io.h>

#define CYCLES_PER_MS 100

void wait_ms(unsigned int time);

/**
 * @brief
 * @param argc
 * @param argv[]
 * @return
 */
int main(int argc, char const *argv[])
{
    /*DDRE = <port direction?>;*/
    /*PORTE = <port output?>;*/

    while (1) {
        wait_ms(250);
        // 	count on port E somehow?

    }

    return 0;
}

/**
 * @brief
 * @param milliseconds
 */
void wait_ms(unsigned int time)
{
    unsigned int cycles = time * CYCLES_PER_MS;

    while (cycles > 0) {
        cycles--;
    }
}

