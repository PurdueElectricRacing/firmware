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

extern q_handle_t q_tx_can;

volatile uint32_t left_ccr = 0;
volatile uint32_t left_update_time = 0;
volatile uint32_t right_ccr = 0;
volatile uint32_t right_update_time = 0;

void wheelSpeedsInit()
{
    // enable interrupts

    // TIM1 (left)
    TIM1->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
    TIM1->CCMR1 |= ((uint32_t) 0b0011) << TIM_CCMR1_IC1F_Pos;
    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1);
    NVIC_SetPriority(TIM1_CC_IRQn, 2);
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
    NVIC_EnableIRQ(TIM1_CC_IRQn);
    PHAL_startTIM(TIM1);

    // TIM2 (right)
    TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
    PHAL_startTIM(TIM2);
}

void wheelSpeedsPeriodic()
{

    uint32_t ccr_store;
    float speed_store;

    ccr_store = left_ccr;
    if (ccr_store != 0 && (sched.os_ticks - left_update_time) < SPEED_TIMEOUT_MS)
    {
        speed_store = FREQ_TO_KPH * (TIM_CLOCK_FREQ / (float) (ccr_store));
        wheel_speeds.left_kph_x100 = (uint16_t) (speed_store * 100);
    }
    else
    {
        wheel_speeds.left_kph_x100 = 0;
    }

    ccr_store = right_ccr;
    if (ccr_store != 0 && (sched.os_ticks - right_update_time) < SPEED_TIMEOUT_MS)
    {
        speed_store = FREQ_TO_KPH * (TIM_CLOCK_FREQ / (float) (ccr_store));
        wheel_speeds.right_kph_x100 = (uint16_t) (speed_store * 100);
    }
    else
    {
        wheel_speeds.right_kph_x100 = 0;
    }

    // ccr_store = right_ccr;
    // if(ccr_store == 0)
    // {
    //     wheel_speeds.right.freq_hz = 0;
    // } 
    // else
    // {
    //     if(ccr_store == 0)
    //         asm("bkpt");
    //     wheel_speeds.right.freq_hz = (FREQ_TO_KPH * TIM_CLOCK_FREQ) / (float) ccr_store;
    //     if(ccr_store == 0)
    //         asm("bkpt");

    // }
    // SEND_WHEEL_SPEEDS(q_tx_can, wheel_speeds.left.freq_hz, wheel_speeds.right.freq_hz);
}

volatile uint32_t left_ccr_msb_counter = 0;
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
    left_ccr = (left_ccr_msb_counter << 16) | TIM1->CCR1;
    left_ccr_msb_counter = 0;
    left_update_time = sched.os_ticks;
    TIM1->SR = ~(TIM_SR_CC1IF);
}

void  TIM2_IRQHandler()
{
    // CCxIF can be cleared by software by reading the captured data stored in the TIMx_CCRx register
    if (TIM2->SR & TIM_SR_UIF)
    {
        right_ccr = 0;
        TIM2->SR = ~(TIM_SR_UIF);
    }
    else if (TIM2->SR & TIM_SR_CC1IF)
    {
        right_ccr = TIM2->CCR1;
        right_update_time = sched.os_ticks;
        TIM2->SR = ~(TIM_SR_CC1IF);
    }
}