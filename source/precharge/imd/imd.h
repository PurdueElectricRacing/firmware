/**
* @file imd.h
* @author Michael Gliane (mgliane@purdue.edu)
* @brief
* @version 0.1
* @date 2023-9-30
* 
* @copyright Copyright (c) 2023
*
*/

#ifndef _PHAL_IMD_H
#define _PHAL_IMD_H

#include <stdbool.h>
#include "stm3214xx.h"

typedef enum {
    ITR0    = 0b000,
    ITR1    = 0b001,
    ITR2    = 0b010,
    ITR3    = 0b011,
    TI1F_ED = 0b100,
    TI1FP1  = 0b101,
    TI2FP2  = 0b110,
    ETRF    = 0b111
} TimerTriggerSelection_t;

typedef enum {
    CC1 = 1,
    CC2 = 2,
    CC3 = 3,
    CC4 = 4
} TimerCCRegister_t;

/**
* @brief                enables and sets the proper GPIO pins (GPIOB pins 5-7) to read IMD channels
* 
*/
void setup_IMDReading();

/**
* @brief Enabling and configuring Timer 3 and 4 to read IMD frequency
*/
void PHAL_setupTIMClk(TIM_TypeDef* timer);

/**
* @brief                Checks if the IMD's OKhs signal is high, signaling that there
*                       is no faulty resistance
*
*/
bool checkIMD_signal_OKhs();

#endif //_PHAL_IMD_H