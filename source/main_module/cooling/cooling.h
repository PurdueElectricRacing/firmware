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

//45 C  (off fail state) bat
//90 C  (drivetrain)
// temp exceeds, stop drive don't open sdc
// room temp based
#define THERM_MUX_END_IDX   (6)

// Cooling Loops

#define COOL_LOOP_START_IDX (2)

/* DRIVETRAIN COOLANT SYSTEM */
#define DT_ERROR_TEMP_C     (100) // temp to not drive

/* BATTERY COOLANT SYSTEM */
#define BAT_ERROR_TEMP_C    (60) // temp to not drive

// Thermistor specifications
#define THERM_R1  10000 // Top resistor in voltage divider
#define MAX_THERM 4095  // 12-bit adc precision
// Temp (Celcius) = a * ln(resistance) + b
// Thermistor PN: GE-2102
#define THERM_A (-25.16f)
#define THERM_B (260.93f)


// Powertrain
// Thermistor PN: GE-1337
#define DT_THERM_A (-22.93f)
#define DT_THERM_B (241.39f)

#define AVG_WINDOW_SIZE 10



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

    bool    daq_override;          // Outputs controlled by DAQ

    uint8_t dt_temp_error; // DT either over temp or not receiving

    uint8_t bat_temp_error;// BAT either over temp or not receiving temps
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