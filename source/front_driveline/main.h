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

//Shockpot Calibration
#define POT_VOLT_MAX_L   5.0f
#define POT_VOLT_MIN_L   3.3f
#define POT_VOLT_MAX_R   5.0f
#define POT_VOLT_MIN_R   3.3f
#define POT_MAX_DIST     75
#define POT_DIST_DROOP_L 56
#define POT_DIST_DROOP_R 56

typedef struct __attribute__((packed)) {
    uint16_t shock_left;
    uint16_t shock_right;
} raw_adc_values_t;

volatile extern raw_adc_values_t raw_adc_values;

// Shock Pots
#define SHOCK_POT_L_GPIO_Port (GPIOA)
#define SHOCK_POT_L_Pin       (0)
#define SHOCK_POT_L_ADC_CH    (0)
#define SHOCK_POT_R_GPIO_Port (GPIOA)
#define SHOCK_POT_R_Pin       (1)
#define SHOCK_POT_R_ADC_CH    (1)

void canTxSendToBack(CanMsgTypeDef_t* msg);
