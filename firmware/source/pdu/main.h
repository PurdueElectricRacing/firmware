#ifndef MAIN_H
#define MAIN_H

/**
 * @file main.h
 * @brief "PDU" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include "common/phal_G4/gpio/gpio.h"

// Status Indicators
#define HEARTBEAT_LED_PORT  (GPIOC)
#define HEARTBEAT_LED_PIN   (15)
#define ERROR_LED_PORT      (GPIOC)
#define ERROR_LED_PIN       (13)
#define CONNECTION_LED_PORT (GPIOC)
#define CONNECTION_LED_PIN  (14)

#endif // MAIN_H
