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
#include "common/phal_L4/tim/tim.h"
#include "common/psched/psched.h"
#include "common/common_defs/common_defs.h"

// Local defines
static WheelSpeeds_t *_ws = 0;

// Local prototypes
static void configEncoder(TIM_TypeDef* tim, bool inverted);

// Local variables
static volatile uint32_t left_update_time = 0;
static volatile uint32_t right_update_time = 0;

/**
 * @brief Configures timers for quadrature encoder operation
 *        Assumes channels A and B are on CH1 and CH2
 * 
 * @param ws    Initialized wheel speed handle
 * @return      True on success, False on fail
 */
bool wheelSpeedsInit(WheelSpeeds_t *ws)
{
    _ws = 0;
    if (!PHAL_enableTIMClk(ws->l->tim)) return false;
    if (!PHAL_enableTIMClk(ws->r->tim)) return false;
    configEncoder(ws->l->tim, ws->l->invert);
    configEncoder(ws->r->tim, ws->r->invert);
    PHAL_startTIM(ws->l->tim);
    PHAL_startTIM(ws->r->tim);
    _ws = ws;   // Set last to prevent periodic from running if error during initialization
    return true;
}

/**
 * @brief Configures a timer for quadrature encoder operation
 *        assumes the corresponding timer clock has already 
 *        been enabled. The inversion is done by inverting
 *        the polarity on CC1. 
 *        Note: currently setup for use with TIM2 and TIM5
 *        Warning: DOES NOT ENABLE
 * 
 * @param tim The timer to configure
 */
static void configEncoder(TIM_TypeDef* tim, bool inverted)
{
    tim->CR1   &= ~TIM_CR1_CEN;                     // Disable counter during configuration
    tim->PSC   =  0;                                // Do not scale the counter clock
    tim->CNT   =  0;                                // Reset the counter value
    tim->ARR   =  0xFFFFFFFF;                       // Set auto reload to allow for maximum counter value
    tim->SMCR  |= (0b0011 << TIM_SMCR_SMS_Pos);     // Encoder mode 3 - counts up/down on TI1FP1 and TI2FP2
    tim->CCMR1 |= (0b01 << TIM_CCMR1_CC1S_Pos) |    
                  (0b01 << TIM_CCMR1_CC2S_Pos);     // Map IC1/2 on TI1/2
    //tim->CCMR1 = 0b01;
    //tim->CCMR2 = 0b01;
    tim->CCER  &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P); // Clear any inversion
    if (inverted) tim->CCER |= TIM_CCER_CC1P;       // Invert CC1 if desired
    // TODO: configure input filter if necessary
    //tim->CCER = 0b00;
}

/**
 * @brief Updates radians per second calculation
 * 
 */
void wheelSpeedsPeriodic()
{
    uint32_t cnt_l, cnt_r, ms;
    float rad_s_cnt;
    if (!_ws) return;               // Only run if initialized

    cnt_l = _ws->l->tim->CNT;        // Record counter values
    cnt_r = _ws->r->tim->CNT;
    ms = sched.os_ticks;            // Record measurement time

    // rad / s = (count / ms) * (1000 ms / s) * (1 / counts_per_rev) * (2PI rad / rev)
    rad_s_cnt = 2.0 * PI / WHEEL_COUNTS_PER_REV / (ms - _ws->last_update_ms) * 1000.0f;
    _ws->l->rad_s = (cnt_l - _ws->l->last_count) * rad_s_cnt;
    _ws->r->rad_s = (cnt_r - _ws->r->last_count) * rad_s_cnt;

    // Update last state values
    _ws->last_update_ms = ms;
    _ws->l->last_count = cnt_l;
    _ws->r->last_count = cnt_r;
}