/**
 * @file main.h
 * @author Anya Pokrovskaya (apokrovs@Purdue.edu)
 * @brief  Software to measure driveline sensors
 * @version 0.1
 * @date 2025-10-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <stdint.h>
#include "common/phal/can.h"
#include "common/phal/gpio.h"

//Shockpot Calibration
#define POT_VOLT_MAX_L   5.0f
#define POT_VOLT_MIN_L   3.3f
#define POT_VOLT_MAX_R   5.0f
#define POT_VOLT_MIN_R   3.3f
#define POT_MAX_DIST     75
#define POT_DIST_DROOP_L 56
#define POT_DIST_DROOP_R 56


// Shock Pots
#define SHOCK_POT_L_GPIO_Port (GPIOA)
#define SHOCK_POT_L_Pin       (0)
#define SHOCK_POT_L_ADC_CH    (0)
#define SHOCK_POT_R_GPIO_Port (GPIOA)
#define SHOCK_POT_R_Pin       (1)
#define SHOCK_POT_R_ADC_CH    (1)

// Normal Force  DY510 Vs- 24V
#define LOAD_FL_GPIO_Port (GPIOB)
#define LOAD_FL_Pin       (0)
#define LOAD_FL_ADC_CH    (8)
#define LOAD_FR_GPIO_Port (GPIOB)
#define LOAD_FR_Pin       (1)
#define LOAD_FR_ADC_CH    (9)
#define LOAD_VOLT_MAX     5.0f
#define LOAD_CELL_CALIBRATION 200.0f
// Magnometer
// #define MAG_FRONT_Port (GPIOB)
// #define MAG_FRONT_Pin (7)

// IR Temp INFKL-800
#define BREAK_TEMP_L_GPIO_Port (GPIOA)
#define BREAK_TEMP_L_Pin       (2)
#define BREAK_TEMP_L_ADC_CH    (2)
#define BREAK_TEMP_R_GPIO_Port (GPIOA)
#define BREAK_TEMP_R_Pin       (3)
#define BREAK_TEMP_R_ADC_CH    (3)


void canTxSendToBack(CanMsgTypeDef_t* msg);

typedef struct __attribute__((packed)) {
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t load_left;
    uint16_t load_right;
    uint16_t shock_left;
    uint16_t shock_right;
    uint16_t break_temp_left;
    uint16_t break_temp_right;

} raw_adc_values_t;

volatile extern raw_adc_values_t raw_adc_values;
