/**
 * project: Lab 2, part 3b
 * @file part3b.c
 * @brief Implement a binary counter that increments 2 sets of leds.
 *        The board LEDs increment at the specified rate while the
 *        external LEDs increment at 1/3rd the rate.
 *        This counter increments when Button 0 on the external
 *        input is pressed, detected by a pin interrupt.
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-13
 * build: 1.0
 */
#include <avr/io.h>
#include <avr/interrupt.h>

/*Define our own boolean values for readability*/
#define FALSE 0
#define TRUE 1

/*Global value to indicate button press*/
volatile short int isPressedBut0 = FALSE;

/**
 * @brief When a pin value is changed, handle that change
 * @param INT4_vect The specified interrupt for pin change
 */
ISR(INT4_vect)
{
    /*check if any button combination including button 0 is pushed*/
    if (PINC & 0x01)
    {
        isPressedBut0 = TRUE;
    }
}

/**
 * @brief Main program loop.  Repeatedly counts from 0-15 using the onboard
 *        and offboard LEDs as indicators.  Offboard counts at 1/3rd the rate.
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 */
int main(int argc, char const *argv[])
{
    isPressedBut0 = FALSE;
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

    /*Set the interrupt to the specified pins*/
    EIMSK = 0x08;
    EIFR = 0x10;

    sei();

    /* Run continuously */
    while (1) {
        /*If the button is pressed, increment counters*/
        if (isPressedBut0 == TRUE)
        {
            PORTE += 0x10;

            /*Only increment the offboard counter every 3rd count*/
            if (divider > 2)
            {
                PORTD += 0x01;
                divider = 0;
            }

            divider++;
            /*we have handled the button press, so reset it*/
            isPressedBut0 = FALSE;
        }
    }

    return 0;
}
