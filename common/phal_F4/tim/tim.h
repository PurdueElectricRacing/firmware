/**
 * @file tim.h
 * @author Luke Oxley (lcoxley@purdue.edu)
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
    CC1 = 1,
    CC2 = 2,
    CC3 = 3,
    CC4 = 4
} TimerCCRegister_t;

typedef enum {
    CC_OUTPUT   = 0b00,
    CC_INTERNAL = 0b01,
    CC_EXTERNAL = 0b10,
    CC_TRC      = 0b11
} TimerInputMode_t;


bool PHAL_enableTIMClk(TIM_TypeDef* timer);
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
 * @brief                   Initializes a timer channel based on configured parameters.
 * 
 * @param timer             The timer to be used
 * @param chnl              The capture compare register to setup 
 * @param input_source      The source for the capture compare register 
 * @param is_falling        Set to false if you care about rising edges 
 * @return                  Returns true if successful
 */
bool PHAL_initPWMChannel(TIM_TypeDef* timer, TimerCCRegister_t chnl, TimerInputMode_t input_source, bool is_falling);

/**
 * @brief       Starts the timer counter
 * 
 * @param timer The timer to start
 */
void PHAL_startTIM(TIM_TypeDef* timer);

/**
 * @brief                Allows for output of a PWM signal, consult reference manual
 *                       for calculating ARR and CCMR1 to configure frequency and period 
 *                       as well as pay attention to PWM resolution.
 * 
 * @param timer          The timer to be used
 * @param counter_period ARR value that configures period
 * @param ccmr1          CCMR1 that configures duty cycle 
 * @param prescaler      Prescaler for input clock 
 * @return               Returns true if successful
 */
bool PHAL_initPWMOut(TIM_TypeDef* timer, uint16_t counter_period, uint16_t ccmr1, uint16_t prescaler);

#endif // _PHAL_TIM_H