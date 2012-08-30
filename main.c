/**
 * project: Lab 1, parts 1 & 2
 * @file main.c
 * @brief
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-08-30
 * build:
 */
#include <avr/io.h>

#define CYCLES_PER_MS 10000000

void wait_ms(unsigned int time);

/**
 * @brief
 * @param argc
 * @param argv[]
 * @return
 */
int main(int argc, char const *argv[])
{
    DDRE = 0xF0;
    PORTE = 0x00;

    while (1) {
        PORTE += 0x10;
        wait_ms(1000);
    }

    return 0;
}

/**
 * @brief
 * @param milliseconds
 */
void wait_ms(unsigned int time)
{
    volatile unsigned int cycles = time * CYCLES_PER_MS;

    while (cycles > 0) {
        cycles--;
    }
}

