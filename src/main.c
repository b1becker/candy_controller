#include <stm32l432xx.h>
#include "ee14lib.h"
#include <stdio.h>
#include <string.h>



/* Pin Definitions: */
#define Echo A0
#define Trig A1
#define TX_UART D4
#define RX_UART D5

/* Functions Declarations: */
void SysTick_Handler(void);
void SysTick_initialize(void);
void delay_ms(int delay);


/* Global Variable Declarations: */

volatile unsigned int counter;




int main()
{
    //Initalize Serial Communication and SysTick
    host_serial_init();
    SysTick_initialize();
    char buffer[200];

    /* Trig & Echo Config */
    gpio_config_mode(Echo, INPUT);
    gpio_config_mode(Trig, OUTPUT);


    gpio_config_mode(A3, OUTPUT);
    volatile echo_signal;

    while(1) {
        //Send out 10 microsecond signal
        gpio_write(Trig, 1);
        delay_ms(1);
        gpio_write(Trig, 0);

        while (gpio_read(Echo) == 0);
        //Read from echo
        volatile int start_time = counter;
        while (gpio_read(Echo) != 0);
        volatile int end_time = counter;

        //Time elapsed in 10 us
        volatile float pulse_time = (end_time - start_time);

        //Distance = speed * time adjusted
        volatile float distance = (pulse_time * 0.343) / 2;
        sprintf(buffer, "Distance: %f\n", distance);
        serial_write(USART2, buffer, strlen(buffer));
        
        delay_ms(10000);
        if(distance < 10.0f) {
            gpio_write(A3, 1);            
        } else {
            gpio_write(A3, 0);
        }
    }
}


void SysTick_Handler(void)
{
    counter++;
}

void SysTick_initialize(void)
{
    SysTick->CTRL = 0;  //Enables the SysTick timer
    // Reload value can be anything in the range 0x00000001-0x00FFFFFF. Setting
    // it to 4000 gives us a frequency of 1kHz.
    SysTick->LOAD = 39;

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

void delay_ms(int ds) {
    counter = 0;
    while (counter != ds) {}
}

//Overloads printf so it can be used normally.
int _write(int file, char *data, int len) {
    serial_write(USART2, data, len);
    return len;
}

//Overloads printf so it can be used normally.
int _write(int file, char *data, int len) {
    serial_write(USART2, data, len);
    return len;
}
// https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/
