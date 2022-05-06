/**
 * @file wheel_speeds.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _WHEEL_SPEEDS_H
#define _WHEEL_SPEEDS_H

#include <stdbool.h>
#include "common/psched/psched.h"
#include "common/phal_L4/tim/tim.h"
#include "stm32l4xx.h"

/**
 * For a clock of 16 MHz, an estimate for
 * resolution can be performed using
 * 32 teeth and wheel diam of 1.5 ft at
 * 55 mph. The model requires a minimum
 * resolution of 0.02 km/hr. A prescaler
 * of 8 achieves this resolution, meaning
 * the timer should be clocked at 
 * a minimum of 2 MHz.
 */
#define TIM_CLOCK_FREQ 2000000

#define WHEEL_DIAM_M 0.4572
#define WHEEL_CIRCUMF_M (3.14159 * WHEEL_DIAM_M)
#define SENSOR_TEETH 32
#define MPS_TO_KPH 3.6
#define FREQ_TO_KPH (MPS_TO_KPH * WHEEL_CIRCUMF_M / SENSOR_TEETH)

#define SPEED_TIMEOUT_MS 300 // Set to 0 at speeds under 1 kph

typedef struct {
  uint16_t left_kph_x100;
  uint16_t right_kph_x100;
} WheelSpeeds_t;

extern WheelSpeeds_t wheel_speeds;

/** 
 *  @brief Configures interrupts and enables timers 
 */ 
void wheelSpeedsInit();
/**
 * @brief Converts timer data to wheel speeds
 * 
 */
void wheelSpeedsPeriodic();


#endif // _WHEEL_SPEEDS_H