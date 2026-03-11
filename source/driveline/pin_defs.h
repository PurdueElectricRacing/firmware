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

// Load Cells
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

// Brake Temps
#define BRAKE_TEMP_L_GPIO_Port (GPIOB)
#define BRAKE_TEMP_L_Pin       (2)
#define BRAKE_TEMP_L_ADC_CH    (12)

#define BRAKE_TEMP_R_GPIO_Port (GPIOB)
#define BRAKE_TEMP_R_Pin       (11)
#define BRAKE_TEMP_R_ADC_CH    (14)

// Oil Temps

#define OIL_TEMP_L_GPIO_Port (GPIOA)
#define OIL_TEMP_L_Pin       (2)
#define OIL_TEMP_L_ADC_CH    (3)

#define OIL_TEMP_R_GPIO_Port (GPIOA)
#define OIL_TEMP_R_Pin       (1)
#define OIL_TEMP_R_ADC_CH    (2)

// Brake Pressure
#define BRAKE_PRESSURE_R_GPIO_Port (GPIOA)
#define BRAKE_PRESSURE_R_Pin       (3)
#define BRAKE_PRESSURE_R_ADC_CH (4)

#define BRAKE_PRESSURE_L_GPIO_Port (GPIOA)
#define BRAKE_PRESSURE_L_Pin       (4)
#define BRAKE_PRESSURE_L_ADC_CH (17)


// Water Temps
#define WATER_TEMP_FL_GPIO_Port (GPIOA)
#define WATER_TEMP_FL_Pin       (5)
#define WATER_TEMP_FL_ADC_CH (13)

#define WATER_TEMP_RL_GPIO_Port (GPIOA)
#define WATER_TEMP_RL_Pin       (6)
#define WATER_TEMP_RL_ADC_CH (3)

#define WATER_TEMP_FR_GPIO_Port (GPIOA)
#define WATER_TEMP_FR_Pin       (7)
#define WATER_TEMP_FR_ADC_CH (4)

#define WATER_TEMP_RR_GPIO_Port (GPIOB)
#define WATER_TEMP_RR_Pin       (0)
#define WATER_TEMP_RR_ADC_CH (12)

// Ambient Temp
#define AMB_TEMP_GPIO_Port (GPIOB)
#define AMB_TEMP_Pin       (1)
#define AMB_TEMP_ADC_CH (1)

// CAN
#define VCAN_RX_GPIO_Port (GPIOB)
#define VCAN_RX_Pin       (5)
#define VCAN_TX_GPIO_Port (GPIOB)
#define VCAN_TX_Pin       (6)


#endif // PIN_DEFS_H
