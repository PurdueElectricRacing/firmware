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
#include "common/common_defs/common_defs.h"
#include "common/faults/faults_common.h"
#include "common/psched/psched.h"

/* Module Includes */
#include <stdint.h>

#define MAX_PEDAL_MEAS (4095)

#define APPS_BRAKE_THRESHOLD               (307) // EV.5.7.1 (7.5% of 0x0FFF)
#define APPS_THROTTLE_FAULT_THRESHOLD      (614) // EV.5.7.1 (25% of 0x0FFF) - making 15 percent in order to prevent premature BSPD trip
#define APPS_THROTTLE_CLEARFAULT_THRESHOLD (205) // EV.5.7.2 ( 5% of 0x0FFF)

#define PROFILES_START_SECTOR (3)
#define NUM_PROFILES          (4)
#define PROFILE_WRITE_SUCCESS (0)
#define PROFILE_WRITE_FAIL    (-1)

typedef struct
{
    bool apps_faulted; // wiring or 10% dev
    bool apps_implaus_detected;
    uint32_t apps_implaus_start_time;
    bool bse_faulted; // wiring
    bool bse_wiring_fail_detected;
    uint32_t bse_wiring_fail_start_time;
    bool apps_brake_faulted; // throttle and brake pressed together
} pedal_faults_t;

typedef struct {
    uint16_t t1_min;
    uint16_t t1_max;
    uint16_t t2_min;
    uint16_t t2_max;
    uint16_t b1_min;
    uint16_t b1_max;
    uint16_t b2_min;
    uint16_t b2_max;
} pedal_calibration_t;

typedef struct {
    uint16_t throttle;
    uint16_t brake;
} pedal_values_t;

typedef struct {
    uint8_t id;
    uint8_t brake_travel_threshold;
    uint8_t throttle_travel_threshold;
    uint8_t reserved;
} driver_pedal_profile_t;

extern pedal_faults_t pedal_faults;
extern pedal_calibration_t pedal_calibration;
extern pedal_values_t pedal_values;
extern uint16_t thtl_limit;
extern driver_pedal_profile_t driver_pedal_profiles[4];

/* Function Prototypes */
void pedalsPeriodic(void);
int writePedalProfiles(void);
void readPedalProfiles(void);

#endif
