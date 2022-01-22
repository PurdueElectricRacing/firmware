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
    //uint32_t left_counts = (total_overflows << 16) | left_ccr;
    wheel_speeds.left.freq_hz = CLK / TIM_PRESC / left_ccr;
    wheel_speeds.right.freq_hz = CLK / TIM_PRESC / right_ccr;
}

uint16_t temp_overflow_counter = 0;
void TIM1_UP_TIM16_IRQHandler()
{
    temp_overflow_counter=1;//++
    TIM1->SR = ~TIM_SR_UIF;
}

uint32_t funky_counter_XD = 0;
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
}