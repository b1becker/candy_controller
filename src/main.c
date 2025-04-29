#include <stm32l432xx.h>
#include "ee14lib.h"
#include <stdio.h>
#include "dfp.h"
#include <string.h>

/* Distance limit Definition */
#define Dist_Range 15

/* Pin Definitions: */
#define Echo1 A1
#define Trig1 A0
#define Echo2 D11
#define Trig2 D12


/* Command Definitions */
#define DEFAULT 0
#define PLAY_NEXT 1
#define PLAY_PREV 2
#define PAUSE 3
#define PLAY 4

/* Functions Declarations: */
void SysTick_Handler(void);
void SysTick_initialize(void);
void delay_10us(int delay);
void usart_send_command(uint8_t cmd);
void usart_select_sd(void);
float get_distance(EE14Lib_Pin trig, EE14Lib_Pin echo);

/* Global Variable Declarations: */
volatile unsigned int counter;


int main()
{
    //Initalize Serial Communication and SysTick
    host_serial_init();
    SysTick_initialize();

    char buffer[200];

    /* Trig & Echo Config */
    gpio_config_mode(Echo1, INPUT);
    gpio_config_mode(Trig1, OUTPUT);
    gpio_config_mode(Echo2, INPUT);
    gpio_config_mode(Trig2, OUTPUT);

    host_uart_init();
    usart_select_sd();
    volatile bool paused = false;
    delay_10us(100000);
    
    usart_send_command(DEFAULT);


    while(1) {
        volatile float distance = get_distance(Trig1, Echo1);
        volatile float distance2 = get_distance(Trig2, Echo2);
        sprintf(buffer, "Distance 1: %f Distance 2: %f\n", distance, distance2);
        serial_write(USART2, buffer, strlen(buffer));
        
        if ((distance <= Dist_Range || distance2 <= Dist_Range) && (paused == true))
        {
            usart_send_command(PLAY);
            paused = false;
            sprintf(buffer, "Unpause: ");
            serial_write(USART2, buffer, strlen(buffer));
        }
        if ((distance > Dist_Range && distance2 > Dist_Range) && (paused == false))
        {
            usart_send_command(PAUSE);
            paused = true;
            sprintf(buffer, "Pause: ");
            serial_write(USART2, buffer, strlen(buffer));
        }
        
        delay_10us(10000);

    }
}

float get_distance(EE14Lib_Pin trig, EE14Lib_Pin echo)
{
    //Send out 10 microsecond signal
    gpio_write(trig, 1);
    delay_10us(1);
    gpio_write(trig, 0);

    while (gpio_read(echo) == 0);
    //Read from Echo1
    volatile int start_time = counter;
    while (gpio_read(echo) != 0);
    volatile int end_time = counter;

    //Time elapsed in 10 us
    volatile float pulse_time = (end_time - start_time);

    //Distance = speed * time adjusted
    volatile float distance = (pulse_time * 0.343) / 2;

    return distance;
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

void delay_10us(int ds) {
    counter = 0;
    while (counter != ds) {}
}

void usart_send_command(uint8_t cmd) {
    //Play first track
    if (cmd == DEFAULT)
    {
        serial_write(USART1, (const char*)zero_buffer, 10);
    }
    //Skip
    if(cmd == PLAY_NEXT) {
        // serial_write(USART1, (const char*)write_buffer, 10);
        serial_write(USART1, (const char*)skip_buffer, 10);
        delay_10us(100000);
        serial_write(USART1, (const char*)skip_buffer, 10);
    }
    //Seek
    else if(cmd == PLAY_PREV) {
        serial_write(USART1, (const char*)seek_buffer, 10);
    }
    //Pause 
    else if (cmd == PAUSE)
    {
        serial_write(USART1, (const char*)pause_buffer, 10);
        delay_10us(100000);
        serial_write(USART1, (const char*)pause_buffer, 10);
    }
    else if (cmd == PLAY)
    {
        serial_write(USART1, (const char*)play_buffer, 10);
        delay_10us(100000);
        serial_write(USART1, (const char*)play_buffer, 10);
    }
}

void usart_select_sd(void) {
    const uint8_t buf[10] = {
        0x7E,       // Start
        0xFF,       // Version
        0x06,       // Length
        0x09,       // CMD = select device
        0x00,       // No feedback
        0x00, 0x02, // Param = 0x0002 (SD card)
        0xFE, 0xF0, // Checksum (0xFEF0)
        0xEF        // End
    };
    serial_write(USART1, (char*)buf, 10);
}

