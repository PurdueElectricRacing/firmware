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

#include "can_parse.h"
#include "common/faults/faults.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/plettenberg/plettenberg.h"
#include "common/psched/psched.h"
#include "cooling.h"
#include "main.h"
#include <stdbool.h>

#define BUZZER_DURATION_MS 2500 // EV.10.5: 1-3s

#define ERROR_FALL_MS (5000)

/* BRAKE LIGHT CONFIG */
#define BRAKE_LIGHT_ON_THRESHOLD  (500)
#define BRAKE_LIGHT_OFF_THRESHOLD (300)

#define BRAKE_PRESSED_THRESHOLD (BRAKE_LIGHT_ON_THRESHOLD)
typedef struct
{
    float torque_left;    // [-100.0, 100.0]
    float torque_right;
} torqueRequest_t;

typedef enum
{
    CAR_TORQUE_NONE,
    CAR_TORQUE_RAW,
    CAR_TORQUE_TV,
    CAR_TORQUE_DAQ
} torqueSource_t;

#define HV_LOW_PASS_SIZE (5)
#define HV_V_MC_CAL      (1000)        // V_actual / V_measured * 1000
#define HV_V_BAT_CAL     (1000)        // V_actual / V_measured * 1000
typedef struct
{
    bool pchg_complete;
    bool pchg_error;
    uint16_t  v_mc_filt;                     // volts x10
    uint16_t  v_bat_filt;                    // volts x10
    uint16_t  v_mc_buff[HV_LOW_PASS_SIZE];   // Units are 12-bit adc
    uint8_t   v_mc_buff_idx;
    uint16_t  v_bat_buff[HV_LOW_PASS_SIZE];  // Units are 12-bit adc
    uint8_t   v_bat_buff_idx;
} prechargeStat_t;

typedef struct
{
    car_state_t state;
    motor_t motor_l;
    motor_t motor_r;

    torqueRequest_t torque_r;
    torqueSource_t torque_src;
    bool regen_enabled;

    prechargeStat_t pchg;
    bool start_btn_debounced;
    bool sdc_close;
    bool brake_light;
    bool buzzer;
    uint32_t buzzer_start_ms;
} Car_t;
extern Car_t car;

bool carInit();
void carHeartbeat();
void carPeriodic();
void parseMCDataPeriodic(void);
void calibrateSteeringAngle(uint8_t* success);

#endif