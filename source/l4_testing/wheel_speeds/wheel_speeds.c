/**
 * @file wheel_speeds.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief   Use pwm input capture to measure left and right wheel speeds
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "wheel_speeds.h"

WheelSpeeds_t wheel_speeds;

uint32_t left_ccr = 0;
uint32_t right_ccr = 0;

void wheelSpeedsPeriodic()
{
    wheel_speeds.left.freq_hz = CLK / TIM_PRESC / left_ccr;
    wheel_speeds.right.freq_hz = CLK / TIM_PRESC / right_ccr;
}

void TIM1_CC_IRQHandler()
{
    left_ccr = TIM1->CCR1;
    TIM1->SR = ~(TIM_SR_CC1IF);
    if (TIM1->SR & TIM_SR_UIF)
    {
        left_ccr |= 0x00010000;
        TIM1->SR = ~TIM_SR_UIF;
    }
}

void  TIM2_IRQHandler()
{
    right_ccr = TIM2->CCR1;
    TIM2->SR = ~(TIM_SR_CC1IF);
}