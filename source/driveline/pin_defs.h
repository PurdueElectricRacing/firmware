/**
 * @file pin_defs.h
 * @brief "Driveline" node pin definitions and related constants
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef PIN_DEFS_H
#define PIN_DEFS_H

#include "common/phal/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOB)
#define HEARTBEAT_LED_PIN   (5)
#define ERROR_LED_PORT      (GPIOB)
#define ERROR_LED_PIN       (9)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (4)

// Shockpots
#define SHOCKPOT_LEFT_GPIO_PORT  (GPIOA)
#define SHOCKPOT_LEFT_GPIO_PIN   (7)
#define SHOCKPOT_LEFT_ADC_CHNL   (4)
#define SHOCKPOT_RIGHT_GPIO_PORT (GPIOA)
#define SHOCKPOT_RIGHT_GPIO_PIN  (4)
#define SHOCKPOT_RIGHT_ADC_CHNL  (10)

#endif // PIN_DEFS_H
