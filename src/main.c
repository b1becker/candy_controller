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
        delay(1);
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
        
        delay(10000);
        if(distance < 10.0f) {
            gpio_write(A3, 1);            
        } else {
            gpio_write(A3, 0);
        }
    }
}

// https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/
