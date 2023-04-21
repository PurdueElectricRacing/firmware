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
#include "stm32l4xx.h"

#define WHEEL_DIAM_M (0.4572)
#define WHEEL_CIRCUMF_M (3.14159 * WHEEL_DIAM_M)
#define WHEEL_COUNTS_PER_REV (1056)

typedef struct {
  TIM_TypeDef *tim;             // Timer peripheral
  bool        invert;           // Invert count direction
  uint32_t    last_count;       // Last timer counter value
  float       rad_s;            // Radians per second
} WheelSpeed_t; 
typedef struct { 
  uint32_t     last_update_ms;  // Last update time (off of os_ticks)
  WheelSpeed_t *l;               // Left wheel configuration
  WheelSpeed_t *r;               // Right wheel configuration
} WheelSpeeds_t;

bool wheelSpeedsInit(WheelSpeeds_t *ws);
void wheelSpeedsPeriodic();

#endif // _WHEEL_SPEEDS_H