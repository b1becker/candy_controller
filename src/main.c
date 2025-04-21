#include <stm32l432xx.h>
#include "ee14lib.h"
#include <stdio.h>
#include <string.h>

#define Echo A0
#define Trig A1

volatile unsigned int counter;


void SysTick_Handler(void)
{
    counter++;
}

void SysTick_initialize(void)
{
    SysTick->CTRL = 0;  //Enables the SysTick timer
    // Reload value can be anything in the range 0x00000001-0x00FFFFFF. Setting
    // it to 4000 gives us a frequency of 1kHz.
    SysTick->LOAD = 39; // Sets the SysTick timer's reload value

    //3999 - 1000 us
    //399  - 100  us  
    //39   - 10   us  

    // This sets the priority of the interrupt to 15 (2^4 - 1), which is the
    // largest supported value (aka lowest priority)
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void delay(int ds) {
    counter = 0;
    while (counter != ds) {}
}

int main()
{
    //Initalize Serial Communication and SysTick
    host_serial_init();
    SysTick_initialize();
    char buffer[200];

    //Configure the Trig & Echo
    // Echo - A0
    // Trig - A1
    gpio_config_mode(Echo, INPUT);
    gpio_config_mode(Trig, OUTPUT);
    volatile echo_signal;
    // Do Stuff
    while(1) {
        //Send out 10 microsecond signal
        gpio_write(Trig, 1);
        delay(1);
        gpio_write(Trig, 0);

        while (gpio_read(Echo) == 0);
        //Read from echo
        volatile int start_time = counter;
        volatile int useless_int = 0;
        while (gpio_read(Echo) != 0);
        volatile int end_time = counter;

        volatile int pulse_time = end_time - start_time;
        volatile float distance = (pulse_time * 0.034) / 2;
        
        sprintf(buffer, "Useless int: %d\n", useless_int);
        serial_write(USART2, buffer, strlen(buffer));
        sprintf(buffer, "Distance: %d\n", pulse_time);
        serial_write(USART2, buffer, strlen(buffer));

        delay(10000);
    }
}

// https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/
