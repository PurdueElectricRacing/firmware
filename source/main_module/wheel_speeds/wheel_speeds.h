/**
 * @file wheel_speeds.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief
 * @version 2.0
 * @date 2023-01-21
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _WHEEL_SPEEDS_H
#define _WHEEL_SPEEDS_H

#include <stdbool.h>

#include "stm32f407xx.h"

// NOTE: if timer clock speed changes, use excel spreadsheet
// to determine optimal prescaler

#define WS_TIM_PSC (36)
// #define WS_TIM_PSC (16)
#define WS_COUNTS_PER_REV (42)
#define WS_TIMEOUT_MS     (300)

typedef struct {
    uint16_t left_rad_s_x100; //!< Left wheel sped rad/s * 100
    uint16_t right_rad_s_x100; //!< Right wheel speed rad/s * 100
} WheelSpeeds_t;

extern WheelSpeeds_t wheel_speeds;

bool wheelSpeedsInit();
void wheelSpeedsPeriodic();

#endif // _WHEEL_SPEEDS_H