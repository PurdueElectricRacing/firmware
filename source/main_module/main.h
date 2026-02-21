/**
 * @file main.h
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

#include "common/phal/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOA)
#define HEARTBEAT_LED_PIN   (0)
#define ERROR_LED_PORT      (GPIOA)
#define ERROR_LED_PIN       (5)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (2)

#endif
