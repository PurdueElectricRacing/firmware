/**
 * @file led.h
 * @author Charles Tamer (ctamer@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2024-1-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _LED_H
#define _LED_H

#include <stdbool.h>
#include <stdint.h>

#include "common/phal/gpio.h"
#include "common/phal/spi.h"

#define MAX_NUM_LED 14

// LED Driver Pins (value = TLC59281 OUT pin number)
#define LED_PUMP_1  (0)
#define LED_PUMP_2  (1)
#define LED_DLBK    (2)
#define LED_DLFR    (3)
#define LED_SDC     (4)
#define LED_MAIN    (5)
#define LED_DASH    (6)
#define LED_ABOX    (7)
#define LED_HXFAN   (8)
#define LED_5V_FAN  (9)
#define LED_TV      (10)
#define LED_DAQ     (11)
#define LED_5V_CRIT (12)
#define LED_BLT     (13)

enum LED_state {
    LED_OFF = 0,
    LED_ON,
    LED_BLINK
};

/**
 * @brief Set, clear, or toggle a specified LED pin
 * 
 * @param spi SPI hande
 * @param led LED driver pin
 * @param state Desired state of the LED : SET, CLEAR, or TOGGLE
 * @return true if data transfer is successful
 * @return false if data transfer fails
 */
bool LED_control(int led, enum LED_state state);

/**
 * @brief Periodic LED function that toggles LEDs that need to be toggled
 * 
 * @param
 * @return
 */
void LED_periodic();

/**
 * @brief Get the status of a specified LED pin
 * 
 * @param led LED driver pin
 * @return enum for the LED state
 */
uint8_t get_LED_status(int led);

#endif /* _LED_H */