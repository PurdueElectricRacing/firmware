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
#include <stdbool.h>

#define BAT_PUMP_ON_TEMP_C  35
#define BAT_PUMP_OFF_TEMP_C 30
#define DT_PUMP_ON_TEMP_C   30
#define DT_PUMP_OFF_TEMP_C  35

#define BAT_REFRESH_TIME_S 10
#define DT_REFRESH_TIME_S 5

typedef struct
{

} Cooling_t;

extern Cooling_t cooling;


#endif