/**
 * @file main.h
 * @brief "Abox" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "common/phal/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT (GPIOB)
#define HEARTBEAT_LED_PIN  (5)
#define ERROR_LED_PORT     (GPIOB)
#define ERROR_LED_PIN      (0)

#endif
