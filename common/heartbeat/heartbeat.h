#ifndef HEARTBEAT_H
#define HEARTBEAT_H

/**
 * @file heartbeat.h
 * @brief Shared heartbeat task implementation for status LEDs.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "common/phal/gpio.h"

typedef struct {
    GPIO_TypeDef *heartbeat_port;
    uint32_t heartbeat_pin;
    GPIO_TypeDef *connection_port;
    uint32_t connection_pin;
    GPIO_TypeDef *error_port;
    uint32_t error_pin;
} status_leds_t;

extern void heartbeat_task(status_leds_t *leds);

/**
 * @brief Macro to define a heartbeat task with predefined behavior for status LEDs.
 * 
 * Usage:
 * 1. Ensure that the macros HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, CONNECTION_LED_PORT, CONNECTION_LED_PIN, ERROR_LED_PORT, and ERROR_LED_PIN are defined
 * 2. Init the GPIOS for the LEDs as outputs in your hardware initialization code.
 * 3. Call DEFINE_HEARTBEAT_TASK() in your source file to define the task.
 * 4. Call START_HEARTBEAT_TASK() in your main function after initializing the kernel to start the task.
 */
#define DEFINE_HEARTBEAT_TASK(); \
    status_leds_t status_leds = { \
        .heartbeat_port = HEARTBEAT_LED_PORT, \
        .heartbeat_pin = HEARTBEAT_LED_PIN, \
        .connection_port = CONNECTION_LED_PORT, \
        .connection_pin = CONNECTION_LED_PIN, \
        .error_port = ERROR_LED_PORT, \
        .error_pin = ERROR_LED_PIN \
    }; \
    void heartbeat_wrapper(void) { heartbeat_task(&status_leds); }; \
    DEFINE_TASK(heartbeat_wrapper, 100, osPriorityLow, STACK_512);

#define START_HEARTBEAT_TASK(); START_TASK(heartbeat_wrapper);

#endif // HEARTBEAT_H