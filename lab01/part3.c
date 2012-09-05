/**
 * project: Lab 1, part 3
 * @file part3.c
 * @brief This implementation rapidly flips the output of the specified port,
 *        used for an oscilloscope test of the delay due to a function call.
 *        In this case, Port E on the atmega64, which maps to JC03 on the board
 * @author Cameron Bentley, Brandon Kasa
 * @date 2012-09-04
 * build: 1.0
 */
#include <avr/io.h>

/**
 * @brief Flip the bit on the port on then off
 */
void flip_output()
{
    PORTE = 0x01;
    PORTE = 0x00;
}

/**
 * @brief Continuously flip the output on the specified port
 * @param argc Argument count
 * @param argv[] Argument list
 * @return Error code
 * @note Swap the comment sections to test inline vs function call
 */
int main(int argc, char const *argv[])
{
    /* Set the specified port as an output port*/
    DDRE = 0x01;
    /* Initialize it to 0, or no signal*/
    PORTE = 0x00;

    /*Flip the output bits repeatedly as quickly as possible*/
    while (1) {
        /*Comment the following line and uncomment the two lines below to
         * switch between inline vs function call*/
        flip_output(); 

        /*PORTE = 0x01;
         *PORTE = 0x00;*/
    }

    return 0;
}

