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

uint32_t left_ccr_msb = 0;
uint32_t left_ccr_lsb = 0;
uint32_t right_ccr = 0;

extern q_handle_t q_tx_can;
extern uint32_t APB2ClockRateHz;

void wheelSpeedsInit()
{
    // enable interrupts
    // TIM1 (left)
    TIM1->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1);
    NVIC_SetPriority(TIM1_CC_IRQn, 2);
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
    NVIC_EnableIRQ(TIM1_CC_IRQn);
    PHAL_startTIM(TIM1);

    // TIM2 (right)
    // TIM2->DIER |= TIM_DIER_CC1IE;
    // NVIC_EnableIRQ(TIM2_IRQn);
    // PHAL_startTIM(TIM2);
}

void wheelSpeedsPeriodic()
{
    wheel_speeds.left.freq_hz = APB2ClockRateHz / TIM_PRESC / ((left_ccr_msb << 16) | left_ccr_lsb);
    wheel_speeds.right.freq_hz = APB2ClockRateHz / TIM_PRESC / right_ccr;
    SEND_WHEEL_SPEEDS(q_tx_can, (uint16_t) wheel_speeds.left.freq_hz, (uint16_t) wheel_speeds.right.freq_hz);
}

uint16_t left_ccr_msb_counter = 0;
void TIM1_UP_TIM16_IRQHandler()
{
    if (TIM1->SR & TIM_SR_UIF)
    {
        left_ccr_msb_counter++;
        TIM1->SR = ~(TIM_SR_UIF);
    }
}

void TIM1_CC_IRQHandler()
{
    left_ccr_lsb = TIM1->CCR1;
    left_ccr_msb = left_ccr_msb_counter;
    left_ccr_msb_counter = 0;
    TIM1->SR = ~(TIM_SR_CC1IF);
}
/*
void  TIM2_IRQHandler()
{
    right_ccr = TIM2->CCR1;
    TIM2->SR = ~(TIM_SR_CC1IF);
}*/