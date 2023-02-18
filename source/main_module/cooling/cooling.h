/**
 * @file cooling.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Drivetrain and Battery Temperature Control
 * @version 0.1
 * @date 2022-03-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _COOLING_H_
#define _COOLING_H_

#include "can_parse.h"
#include "car.h"
#include "common/common_defs/common_defs.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/psched/psched.h"
#include <float.h>
#include "main.h"
#include <math.h>
#include <stdbool.h>
#include "stm32l496xx.h"

//45 C  (off fail state) bat
//90 C  (drivetrain)
// temp exceeds, stop drive don't open sdc
// room temp based

#define DT_ALWAYS_COOL  1
#define BAT_ALWAYS_COOL 1
#define DT_FLOW_CHECK_OVERRIDE  1
#define BAT_FLOW_CHECK_OVERRIDE 1

/* DRIVETRAIN COOLANT SYSTEM */
#define DT_PUMP_ON_TEMP_C   (27)
#define DT_PUMP_OFF_TEMP_C  (26)
#define DT_ERROR_TEMP_C     (100) // temp to not drive

// TODO: calibrate flow rates
// Flow checks
#define DT_MAX_FLOW_L_M (26) // average of 3.7
#define DT_MIN_FLOW_L_M (DT_MAX_FLOW_L_M / 3)
#define DT_FLOW_STARTUP_TIME_S (5)

/* BATTERY COOLANT SYSTEM */
#define BAT_PUMP_ON_TEMP_C  (27)
#define BAT_PUMP_OFF_TEMP_C (26)
#define BAT_ERROR_TEMP_C    (60) // temp to not drive

// Flow checks
#define BAT_MAX_FLOW_L_M (26)
#define BAT_MIN_FLOW_L_M (20)
#define BAT_FLOW_STARTUP_TIME_S (5)

// Part 828 on Adafruit
#define PULSES_P_LITER (450)

// Thermistor specifications
#define THERM_R1  10000 // Top resistor in voltage divider
#define MAX_THERM 4095  // 12-bit adc precision
// Temp (Celcius) = a * ln(resistance) + b
#define THERM_A (-25.16)
#define THERM_B (260.93)

#define AVG_WINDOW_SIZE 10

typedef struct
{
    uint8_t  dt_liters_p_min_x10;
    uint8_t  bat_liters_p_min_x10;
    float    dt_therm_out_C;
    float    dt_therm_in_C;
    float    bat_therm_out_C;
    float    bat_therm_in_C;
    uint32_t dt_delta_t;
    uint32_t bat_delta_t;

    uint8_t dt_pump;       // DT pump turned on
    uint8_t dt_fan_power;        // DT fan turned on
    uint8_t dt_temp_error; // DT either over temp or not receiving
    uint8_t dt_flow_error; // DT flow is too low
    uint8_t dt_rose;       // DT pump has been on for
                           // the startup time
    uint8_t bat_pump;      // BAT pump turned on
    uint8_t bat_fan_power;       // BAT fan turned on
    uint8_t bat_temp_error;// BAT either over temp or not receiving temps
    uint8_t bat_flow_error;// BAT flow is too low
    uint8_t bat_rose;      // BAT pump has been on for
                           // the startup time
} Cooling_t;

extern Cooling_t cooling;

/**
 * @brief        Initializes gpio interrupts
 *               Sets default output states
 * @return true  Success
 * @return false Fail
 */
bool coolingInit();

/**
 * @brief Determines
 *        if fans and pumps should be turned
 *        on and checks for faults
 */
void coolingPeriodic();
/**
 * @brief   Converts from a raw thermistor reading to celcius
 *          Based on digikey sensor GE-2102
 * @param t Raw thermistor measurement
 * @return  Degrees Celcius
 */
float rawThermtoCelcius(uint16_t t);

void setFanPWM(void);
static double native_log_computation(const double);
#endif