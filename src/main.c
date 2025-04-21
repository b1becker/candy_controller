#include <stm32l432xx.h>
#include "ee14lib.h"
#include <stdio.h>
#include <string.h>

unsigned int counter;

void SysTick_Handler(void)
{
    counter++;
}

void SysTick_initialize(void)
{
    SysTick->CTRL = 0;  //Enables the SysTick timer
    // Reload value can be anything in the range 0x00000001-0x00FFFFFF. Setting
    // it to 4000 gives us a frequency of 1kHz.
    SysTick->LOAD = 399; // Sets the SysTick timer's reload value

    //3999 - 1000 us
    //399  - 100  us  (ten of these is 10 us, which is what we need to send out)
    //39   - 10   us  

    // This sets the priority of the interrupt to 15 (2^4 - 1), which is the
    // largest supported value (aka lowest priority)
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

int main()
{
    //Initalize Serial Communication and SysTick
    host_serial_init();
    SysTick_initialize();
    char buffer[200];

    //Measure and print
    while(1) {
        //Print
        sprintf(buffer, "meow");
        serial_write(USART2, buffer, strlen(buffer));
    }
}
