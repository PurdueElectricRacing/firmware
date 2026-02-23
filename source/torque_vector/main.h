/**
 * @file main.h
 * @brief "Torque Vector" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

#include "common/phal/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOB)
#define HEARTBEAT_LED_PIN   (4)
#define ERROR_LED_PORT      (GPIOA)
#define ERROR_LED_PIN       (15)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (3)

#endif
