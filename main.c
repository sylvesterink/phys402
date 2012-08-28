#include <avr/io.h> 

void wait_ms(unsigned int milliseconds);

/**
 * @brief 
 * @param argc
 * @param argv[]
 * @return 
 */
int main(int argc, char const *argv[])
{
    wait_ms(1000);
    return 0;
}

/**
 * @brief 
 * @param milliseconds
 */
void wait_ms(unsigned int milliseconds)
{
    int temp=0;
    for (unsigned int i = 0; i < milliseconds; ++i)
    {
        temp++;
    }
}

