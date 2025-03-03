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
#include "main.h"
#include "common/psched/psched.h"
#include "common/common_defs/common_defs.h"


// Local defines
WheelSpeeds_t wheel_speeds;

// Local variables
static volatile uint32_t left_ccr = 0;
static volatile uint32_t left_update_time = 0;
static volatile uint32_t right_ccr = 0;
static volatile uint32_t right_update_time = 0;

/**
 * @brief Configures timers for quadrature encoder operation
 *        Assumes channels A and B are on CH1 and CH2
 *
 * @return      True on success, False on fail
 */
bool wheelSpeedsInit(void)
{
    /* Right Init (TIM 1) */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    MOTOR_R_WS_PWM_TIM->CR1 &= ~TIM_CR1_CEN; // Disable counter (turn off timer)
    MOTOR_R_WS_PWM_TIM->CR1 |= TIM_CR1_URS; 

    MOTOR_R_WS_PWM_TIM->PSC = WS_TIM_PSC - 1;
    MOTOR_R_WS_PWM_TIM->ARR = 0xFFFF;

    // Enable capture/compare and overflow interrupts
    TIM1->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1);
    NVIC_SetPriority(TIM1_CC_IRQn, 2);
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    NVIC_EnableIRQ(TIM1_CC_IRQn);

    /* Set input capture mode */
    MOTOR_R_WS_PWM_TIM->CCER &= ~TIM_CCER_CC1E; // Turn off the channel (necessary to write CC1S bits)

    /* Setup capture compare 1 (period) */
    MOTOR_R_WS_PWM_TIM->CCMR1 &= ~TIM_CCMR1_CC1S;
    MOTOR_R_WS_PWM_TIM->CCMR1 |= TIM_CCMR1_CC1S_0; // Map IC1 to TI1

    /* CCR1 (period) needs rising edge */
    MOTOR_R_WS_PWM_TIM->CCER &= ~TIM_CCER_CC1P;
    MOTOR_R_WS_PWM_TIM->CCER &= ~TIM_CCER_CC1NP;

    /* Select trigger */
    MOTOR_R_WS_PWM_TIM->SMCR &= ~TIM_SMCR_TS;
    MOTOR_R_WS_PWM_TIM->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0;

    /* Select trigger */
    MOTOR_R_WS_PWM_TIM->SMCR &= ~TIM_SMCR_SMS;
    MOTOR_R_WS_PWM_TIM->SMCR |= TIM_SMCR_SMS_2;

    /* Enable channels */
    MOTOR_R_WS_PWM_TIM->CCER |= TIM_CCER_CC1E; // Enable CCR1

    MOTOR_R_WS_PWM_TIM->CR1 |= TIM_CR1_CEN; // Enable timer

    /* Left Init (TIM4) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    MOTOR_L_WS_PWM_TIM->CR1 &= ~TIM_CR1_CEN; // Disable counter (turn off timer)
    MOTOR_L_WS_PWM_TIM->CR1 |= TIM_CR1_URS;


    MOTOR_L_WS_PWM_TIM->PSC = WS_TIM_PSC - 1;
    MOTOR_L_WS_PWM_TIM->ARR = 0xFFFF;

    TIM4->DIER |= TIM_DIER_CC2IE | TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM4_IRQn);

    /* Set input capture mode */
    MOTOR_L_WS_PWM_TIM->CCER &= ~TIM_CCER_CC2E; // Turn off the channel (necessary to write CC1S bits)

    /* Setup capture compare 1 (period) */
    MOTOR_L_WS_PWM_TIM->CCMR1 &= ~TIM_CCMR1_CC2S;
    MOTOR_L_WS_PWM_TIM->CCMR1 |= TIM_CCMR1_CC2S_0; // Map IC2 to TI2

    /* CCR1 (period) needs rising edge */
    MOTOR_L_WS_PWM_TIM->CCER &= ~TIM_CCER_CC2P;
    MOTOR_L_WS_PWM_TIM->CCER &= ~TIM_CCER_CC2NP;

    /* Select trigger */
    MOTOR_L_WS_PWM_TIM->SMCR &= ~TIM_SMCR_TS;
    MOTOR_L_WS_PWM_TIM->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_1;

    /* Select trigger */
    MOTOR_L_WS_PWM_TIM->SMCR &= ~TIM_SMCR_SMS;
    MOTOR_L_WS_PWM_TIM->SMCR |= TIM_SMCR_SMS_2;

    /* Enable channels */
    MOTOR_L_WS_PWM_TIM->CCER |= TIM_CCER_CC2E; // Enable CCR1

    MOTOR_L_WS_PWM_TIM->CR1 |= TIM_CR1_CEN; // Enable timer

    return true;
}

// Conversion:
// (Tooth / Tick) * (Tick / Sec) * (rad / tooth) * 100
// (1 / CCR1) * (72MHz / 36) * (2pi / 42) * 100
// 29919930 / CCR1
#define RAD_S_100_CONVERSION (29919930)

/**
 * @brief Updates radians per second calculation
 *
 */
void wheelSpeedsPeriodic()
{
    uint32_t ccr_store;
    float speed_store;

    ccr_store = left_ccr;

    if (ccr_store != 0 && (sched.os_ticks - left_update_time) < WS_TIMEOUT_MS)
    {
        wheel_speeds.left_rad_s_x100 = RAD_S_100_CONVERSION / ccr_store;
    }
    else
    {
        wheel_speeds.left_rad_s_x100 = 0;
    }

    ccr_store = right_ccr;
    if (ccr_store != 0 && (sched.os_ticks - right_update_time) < WS_TIMEOUT_MS)
    {
        wheel_speeds.right_rad_s_x100 = RAD_S_100_CONVERSION / ccr_store;
    }
    else
    {
        wheel_speeds.right_rad_s_x100 = 0;
    }
}

volatile uint32_t right_ccr_msb_counter = 0;
void TIM1_UP_TIM10_IRQHandler()
{
    if (TIM1->SR & TIM_SR_UIF)
    {
        right_ccr_msb_counter++;
        TIM1->SR = ~(TIM_SR_UIF);
    }
}

void TIM1_CC_IRQHandler()
{
    if (TIM1->SR & TIM_SR_CC1IF)
    {
        right_ccr = (right_ccr_msb_counter << 16) | TIM1->CCR1;
        right_ccr_msb_counter = 0;
        right_update_time = sched.os_ticks;
        TIM1->SR = ~(TIM_SR_CC1IF);
    }
}

volatile uint32_t left_ccr_msb_counter = 0;
void TIM4_IRQHandler()
{
    if (TIM4->SR & TIM_SR_UIF)
    {
        left_ccr_msb_counter++;
        TIM4->SR = ~(TIM_SR_UIF);
    }
    if (TIM4->SR & TIM_SR_CC2IF)
    {
        left_ccr = (left_ccr_msb_counter << 16) | TIM4->CCR2;
        left_ccr_msb_counter = 0;
        left_update_time = sched.os_ticks;
        TIM4->SR = ~(TIM_SR_CC2IF);
    }
}
