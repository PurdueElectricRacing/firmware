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

typedef struct {
  struct {
    uint16_t freq_hz;
  } left;
  struct {
    uint16_t freq_hz;
  } right;
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