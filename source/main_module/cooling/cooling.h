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
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/psched/psched.h"
#include <float.h>
#include "main.h"
#include <math.h>
#include <stdbool.h>


#define THERM_MUX_END_IDX   (6)
#define B_THERM_IDX       (5) // Problem thermistor index

// Cooling Loops

#define COOL_LOOP_START_IDX (2)

// Temp (Celcius) = a * ln(resistance) + b
#define MAX_THERM 4095  // 12-bit adc precision

// Thermistor specifications
#define W_THERM_R1  2200 // Top resistor in voltage divider
#define B_THERM_R1  2000 // There is an error in schematic so this is a software fix to correct for 200ohm difference in top resistor

// Thermistor PN: GE-2102
#define W_THERM_A (-25.16f)
#define W_THERM_B (260.93f)

// Powertrain
// Thermistor PN: GE-1337
#define DT_THERM_A (-23.12f)
#define DT_THERM_B (218.63f)
#define DT_THERM_R1 2200


typedef struct
{
    int8_t    gb_therm_r_c;
    int8_t    gb_therm_l_c;
    int8_t    bat_therm_in_C; //therm 1 (These are drivetrain in altium)
    int8_t    bat_therm_out_C; //therm 2
    int8_t    dt_therm_out_C; //therm 1 (These are Battery in altium)
    int8_t    dt_therm_in_C; //therm 2
    uint32_t dt_delta_t;
    uint32_t bat_delta_t;
} Cooling_t;

extern Cooling_t cooling;

/**
 * @brief        Initializes Thermistor Mux
 * @return true  Success
 * @return false Fail
 */
bool coolingInit();

/**
 * @brief Checks temperature of various cooling loops/Powertrain periodically
 */
void coolingPeriodic();


/**
 * @brief Converts Raw ADC values to Temperature
 *
 * @param t ADC Value
 * @param a 'a' constant in thermistor equation above
 * @param b 'b' constant in thermistor equation above
 * @param r1 Top resistor value for thermistor
 * @return float
 */
float rawThermtoCelcius(uint16_t t, float a, float b, uint16_t r1);

/**
 * @brief Natural log without including math library
 *
 * @param n Value to take natural log of
 * @return double natural log of n
 */
static double native_log_computation(const double n);
#endif