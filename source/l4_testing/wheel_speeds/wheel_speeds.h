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

#define TIM_PRESC 32

#define CLK 3952000.0

typedef struct {
  // left: TIM1  PA8 
  // right: TIM2 PA15
  struct {
    float freq_hz;
  } left;
  struct {
    float freq_hz;
  } right;

} WheelSpeeds_t;

extern WheelSpeeds_t wheel_speeds;


/**
 * @brief Converts timer data to wheel speeds
 * 
 */
void wheelSpeedsPeriodic();


#endif // _WHEEL_SPEEDS_H