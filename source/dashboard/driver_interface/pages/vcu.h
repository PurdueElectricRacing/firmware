#ifndef VCU_H
#define VCU_H

/**
 * @file VCU.h
 * @brief VCU page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#define VCU_STRING "VCU"

// Object names
#define VCU_MODE_BUTTON     "mode"
#define LATERAL_GAIN_BUTTON "lat"
#define LONG_GAIN_BUTTON    "long"
#define EBB_BUTTON          "ebb"
#define REGEN_BUTTON        "regen"
#define TV_BUTTON           "tv"
#define LEFT_WHEEL_BUTTON   "left"
#define RIGHT_WHEEL_BUTTON  "right"

void faults_update(void);
void faults_move_up(void);
void faults_move_down(void);
void faults_select(void);
void faults_telemetry_update(void);

#endif // VCU_H