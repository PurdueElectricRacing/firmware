/**
 * @file pedals.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Read, check, and send pedal measurements
 * @version 0.1
 * @date 2022-03-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _PEDALS_H_
#define _PEDALS_H_

/* System Includes */
#include "common/psched/psched.h"
#include "common/common_defs/common_defs.h"

/* Module Includes */
#include "can_parse.h"

#define MAX_PEDAL_MEAS (4095)

#define APPS_IMPLAUS_MAX_DIFF (410)  // T.4.2.4  (10% of 0x0FFF)
#define APPS_IMPLAUS_TIME_MS  (100)  // T.4.2.5
#define APPS_IMPLAUS_MIN      (205)  // T.4.2.10 ( 5% of 0x0FFF)
#define APPS_IMPLAUS_MAX      (3891) // T.4.2.10 (95% of 0x0FFF)

#define BSE_IMPLAUS_MIN (205)  // ( 5% of 0x0FFF)
#define BSE_IMPLAUS_MAX (3891) // (95% of 0x0FFF)

#define APPS_BRAKE_THRESHOLD               (307)  // EV.5.7.1 (7.5% of 0x0FFF)
#define APPS_THROTTLE_FAULT_THRESHOLD      (1024) // EV.5.7.1 (25% of 0x0FFF)
#define APPS_THROTTLE_CLEARFAULT_THRESHOLD (205)  // EV.5.7.2 ( 5% of 0x0FFF)

#define BREAK_PRESSURE_THRESHOLD (425)

typedef struct
{
    bool     apps_faulted;              // wiring or 10% dev
    bool     apps_implaus_detected;
    uint32_t apps_implaus_start_time;
    bool     bse_faulted;               // wiring
    bool     bse_wiring_fail_detected;
    uint32_t bse_wiring_fail_start_time;
    bool     apps_brake_faulted;        // throttle and brake pressed together
} pedals_t;

extern pedals_t pedals;

typedef struct __attribute__((packed)) 
{
    // Do not modify this struct unless
    // you modify the ADC DMA config 
    // in main.h to match
    uint16_t t1;
    uint16_t t2;
    uint16_t b1;
    uint16_t b2;
    uint16_t b3;
} raw_pedals_t;

volatile extern raw_pedals_t raw_pedals;

typedef struct {
    uint16_t t1max;
    uint16_t t1min;
    uint16_t t2max;
    uint16_t t2min;
    uint16_t b1max;
    uint16_t b1min;
    uint16_t b2max;
    uint16_t b2min;
    uint16_t b3max;
    uint16_t b3min;
} pedal_calibration_t;

extern pedal_calibration_t pedal_calibration;

/* Function Prototypes */
void pedalsPeriodic(void);

#endif