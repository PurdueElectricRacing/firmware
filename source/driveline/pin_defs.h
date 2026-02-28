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
#define HEARTBEAT_LED_PORT  (GPIOA)
#define HEARTBEAT_LED_PIN   (15)
#define ERROR_LED_PORT      (GPIOB)
#define ERROR_LED_PIN       (4)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (3)

// Shockpots
#define SHOCKPOT_LEFT_GPIO_PORT  (GPIOB)
#define SHOCKPOT_LEFT_GPIO_PIN   (13)
#define SHOCKPOT_LEFT_ADC_CHNL   (5)
#define SHOCKPOT_RIGHT_GPIO_PORT (GPIOB)
#define SHOCKPOT_RIGHT_GPIO_PIN  (12)
#define SHOCKPOT_RIGHT_ADC_CHNL  (3)

// Normal Force  DY510 Vs- 24V
#define LOAD_FL_GPIO_Port (GPIOB)
#define LOAD_FL_Pin       (14)
#define LOAD_FL_ADC_CH    (5)

#define LOAD_RL_GPIO_Port (GPIOB)
#define LOAD_RL_Pin       (15)
#define LOAD_RL_ADC_CH    (15)

#define LOAD_FR_GPIO_Port (GPIOA)
#define LOAD_FR_Pin       (8)
#define LOAD_FR_ADC_CH    (1)

#define LOAD_RR_GPIO_Port (GPIOA)
#define LOAD_RR_Pin       (9)
#define LOAD_RR_ADC_CH    (2)

#define LOAD_VOLT_MAX     5.0f
#define LOAD_CELL_CALIBRATION 200.0f
// Magnometer
// #define MAG_FRONT_Port (GPIOB)
// #define MAG_FRONT_Pin (7)

// IR Temp INFKL-800 EITHER PB2 or PB11
#define BRAKE_TEMP_L_GPIO_Port (GPIOB)
#define BRAKE_TEMP_L_Pin       (2)
#define BRAKE_TEMP_L_ADC_CH    (2)

#define BRAKE_TEMP_R_GPIO_Port (GPIOB)
#define BRAKE_TEMP_R_Pin       (11)
#define BRAKE_TEMP_R_ADC_CH    (14)



#endif // PIN_DEFS_H
