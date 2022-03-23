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

#include "stm32l496xx.h"
#include "common/common_defs/common_defs.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/psched/psched.h"
#include <stdbool.h>
#include <math.h>
#include "can_parse.h"
#include "main.h"
#include "car.h"

/* DRIVETRAIN COOLANT SYSTEM */
#define DT_PUMP_ON_TEMP_C   (30)
#define DT_PUMP_OFF_TEMP_C  (35)

// Flow checks
#define DT_MAX_FLOW_L_M (26)
#define DT_MIN_FLOW_L_M (DT_MAX_FLOW_L_M / 3)
#define DT_FLOW_STARTUP_TIME_S (5)

/* BATTERY COOLANT SYSTEM */
#define BAT_PUMP_ON_TEMP_C  (35)
#define BAT_PUMP_OFF_TEMP_C (30)

// Flow checks
#define BAT_MAX_FLOW_L_M (26)
#define BAT_MIN_FLOW_L_M (BAT_MAX_FLOW_L_M / 3)
#define BAT_FLOW_STARTUP_TIME_S (5)

// Part 828 on Adafruit
#define PULSES_P_LITER (450)

// Thermistor specifications
#define THERM_R1  10000 // Top resistor in voltage divider
#define MAX_THERM 4096  // 12-bit adc precision
// Temp (Celcius) = a * ln(resistance) + b
#define THERM_A -25.16
#define THERM_B 260.93

typedef struct
{
    uint8_t dt_liters_p_min;
    uint8_t bat_liters_p_min;
    struct 
    {
        uint8_t dt_pump: 1;       // DT pump turned on
        uint8_t dt_fan: 1;        // DT fan turned on
        uint8_t dt_flow_error: 1; // DT flow is too low
        uint8_t dt_rose: 1;       // DT pump has been on for 
                                  // the startup time
        uint8_t bat_pump: 1;      // BAT pump turned on
        uint8_t bat_fan: 1;       // BAT fan turned on
        uint8_t bat_flow_error: 1;// BAT flow is too low
        uint8_t bat_rose: 1;      // BAT pump has been on for
                                  // the startup time
    };
} Cooling_t;

extern Cooling_t cooling;

/**
 * @brief        Initializes gpio interrupts
 *               Sets default output states
 * @return true  Success
 * @return false Fail
 */
bool initCooling();
/**
 * @brief Calculates flow rates, determines
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
float rawThermtoCelcius(uint16_t t)
#endif