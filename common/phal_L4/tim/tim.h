/**
 * @file tim.h
 * @author Luke Oxleu (lcoxley@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2022-01-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _PHAL_TIM_H
#define _PHAL_TIM_H

#include <stdbool.h>
#include "stm32l4xx.h"

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
    CC1 = 0b00,
    CC2 = 0b01,
    CC3 = 0b10,
    CC4 = 0b11
} TimerCCRegister_t;

typedef enum {
    CC_OUTPUT   = 0b00,
    CC_INTERNAL = 0b01,
    CC_EXTERNAL = 0b10,
    CC_TRC      = 0b11
} TimerInputMode_t;


/**
 * @brief Setup a timer for input capture to measure frequency of a PWM signal
 * 
 * @param timer             The timer to be used
 * @param prescaler         The amount to scale the timer clock by  
 * @param trigger_select    The signal that resets the counter (usually TI1FP1) 
 * @return                  Returns true if successful
 */
bool PHAL_initPWMIn(TIM_TypeDef* timer, uint32_t prescaler, TimerTriggerSelection_t trigger_select);

/**
 * @brief 
 * 
 * @param timer             The timer to be used
 * @param chnl              The capture compare register to setup 
 * @param input_source      The source for the capture compare register 
 * @param is_falling        Set to false if you care about rising edges 
 * @return                  Returns true if successful
 */
bool PHAL_initPWMChannel(TIM_TypeDef* timer, TimerCCRegister_t chnl, TimerInputMode_t input_source, bool is_falling);

#endif // _PHAL_TIM_H