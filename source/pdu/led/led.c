/**
 * @file led.c
 * @author Charles Tamer (ctamer@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2024-1-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "led.h"
#include "main.h"

extern SPI_InitConfig_t spi_config;

uint16_t LED_toggle = 0x0000;


bool LED_control(int led, enum LED_state state)
{
    uint16_t LED_control_data_new = 0x0000;

    static uint16_t LED_control_data_persist;
    if(!LED_control_data_persist)
    {
        LED_control_data_persist = 0x0000;
    }

    /* Get data offset to account for Endianness:
     *
     * SPI transfers the LSB, followed by the MSB. 
     * However, the LED driver wants the MSB transferred first.
     *
     * Therefore, write data for the first 8 LEDs to the second
     * byte and data for the next 8 LEDs to the first byte
    */

    int offset;
    if(led < 8)
    {
        offset = 8;
    }
    else
    {
        offset = -8;
    }

    // Create 16-bit LED control data to write
    switch(state)
    {
        case OFF:       // Clear bit
            LED_control_data_new = LED_control_data_persist &= ~(1 << (led + offset));
            LED_toggle &= ~(1 << led);
            break;
        case ON:        // Set bit
            LED_control_data_new = LED_control_data_persist |= (1 << (led + offset));
            LED_toggle &= ~(1 << led);
            break;
        case BLINK:     // Toggle bit
            LED_control_data_new = LED_control_data_persist ^= (1 << (led + offset));
            LED_toggle |= (1 << led);
            break;
    }

    // Update persisting control data for next time an LED is controlled
    LED_control_data_persist = LED_control_data_new;

    // Transfer LED control data to the ON/OFF Control Shift Register
    bool is_transfer_successful = PHAL_SPI_transfer(&spi_config, (uint8_t*) &LED_control_data_new, 2, NULL);

    for(int i = 0; i < 80; i++)
    {
        asm("nop");
    }

    // Latch LED control data into the ON/OFF Control Data Latch at rising edge of LAT
    PHAL_writeGPIO(LED_CTRL_LAT_GPIO_Port, LED_CTRL_LAT_Pin, 1);

    for(int i = 0; i < 80; i++)
    {
        asm("nop");
    }

    PHAL_writeGPIO(LED_CTRL_LAT_GPIO_Port, LED_CTRL_LAT_Pin, 0);

    // Set Blank low so corresponding output is turned on if data in the ON/OFF control data latch are '1'
    if(PHAL_readGPIO(LED_CTRL_BLANK_GPIO_Port, LED_CTRL_BLANK_Pin))
    {
        PHAL_writeGPIO(LED_CTRL_BLANK_GPIO_Port, LED_CTRL_BLANK_Pin, 0);
    }

    return is_transfer_successful;
}

void LED_periodic()
{
    for(int i = 0; i < 14; i++)
    {
        if(LED_toggle & (1 << i))
        {
            LED_control(i, BLINK);
        }
    }
}