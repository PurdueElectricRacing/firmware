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
#include "can_parse.h"

WheelSpeeds_t wheel_speeds;

uint32_t left_ccr = 0;
uint32_t right_ccr = 0;

//extern q_handle_t q_tx_can;

void wheelSpeedsPeriodic()
{
    wheel_speeds.left.freq_hz = CLK / TIM_PRESC / left_ccr;
    wheel_speeds.right.freq_hz = CLK / TIM_PRESC / right_ccr;
    //SEND_WHEEL_SPEEDS(q_tx_can, (uint16_t) wheel_speeds.left.freq_hz, (uint16_t) wheel_speeds.right.freq_hz);
}

uint16_t left_ccr_msb = 0;
void TIM1_UP_TIM16_IRQHandler()
{
    left_ccr_msb++;
    TIM1->SR = ~(TIM_SR_UIF);
}

void TIM1_CC_IRQHandler()
{
    left_ccr = (left_ccr_msb << 8) | TIM1->CCR1;
    left_ccr_msb = 0;
    TIM1->SR = ~(TIM_SR_CC1IF);
    // if (TIM1->SR & TIM_SR_UIF)
    // {
    //     left_ccr |= 0x00010000;
    //     TIM1->SR = ~TIM_SR_UIF;
    // }
}
/*
void  TIM2_IRQHandler()
{
    right_ccr = TIM2->CCR1;
    TIM2->SR = ~(TIM_SR_CC1IF);
}*/