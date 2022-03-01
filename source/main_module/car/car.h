/**
 * @file car.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Master Car Control and Safety Checking
 * @version 0.1
 * @date 2022-03-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _CAR_H_
#define _CAR_H_

#include <stdbool.h>
#include "main.h"
#include "can_parse.h"

#define BRAKE_PRESSED_THRESHOLD 0.30f
#define BUZZER_DURATION_MS 2000 // EV.10.5: 1-3s

typedef enum
{
    CAR_STATE_INIT = 0,
    CAR_STATE_PREREADY2DRIVE,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER
} CarState_t;

typedef struct
{
    CartState_t state;
} Car_t;

volatile extern Car_t car;

#endif