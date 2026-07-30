/**
 * @file pwm.c
 * @author Natasha Pandit (npandit@purdue.edu)
 * @date 2026-07-25
 */

#include "common/phal_G4/pwm/pwm.h"

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;

bool PHAL_initPWM(TIM_TypeDef* tim, uint32_t frequency_hz, uint8_t channels_en) {
    bool uses_apb2 = false;
    bool requires_main_out_en = false;
    uint8_t max_channels = 0U;

    if (tim == NULL || frequency_hz == 0U) {
        return false;
    }

    switch((uint32_t)tim) {
        // APB2 timers
        case (uint32_t)TIM1:
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
            uses_apb2 = true;
            requires_main_out_en = true;
            max_channels = 4U;
            break;

        case (uint32_t)TIM8:
            RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
            uses_apb2 = true;
            requires_main_out_en = true;
            max_channels = 4U;
            break;

        case (uint32_t)TIM15:
            RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
            uses_apb2 = true;
            requires_main_out_en = true;
            max_channels = 2U;
            break;

        case (uint32_t)TIM16:
            RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
            uses_apb2 = true;
            requires_main_out_en = true;
            max_channels = 1U;
            break;

        case (uint32_t)TIM17:
            RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
            uses_apb2 = true;
            requires_main_out_en = true;
            max_channels = 1U;
            break;

        case (uint32_t)TIM20:
            RCC->APB2ENR |= RCC_APB2ENR_TIM20EN;
            uses_apb2 = true;
            requires_main_out_en = true;
            max_channels = 4U;
            break;

        // APB1 timer
        case (uint32_t)TIM2:
            RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
            max_channels = 4U;
            break;

        case (uint32_t)TIM3:
            RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
            max_channels = 4U;
            break;
        
        case (uint32_t)TIM4:
            RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
            max_channels = 4U;
            break;

        case (uint32_t)TIM5:
            RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
            max_channels = 4U;
            break;   
            
        case (uint32_t)TIM6:
        case (uint32_t)TIM7:

        default:
            return false;
    }

    if (channels_en == 0U || channels_en > max_channels) {
        return false;
    }

    tim->CR1 &= ~TIM_CR1_CEN;

    /*
     * PWM frequency:
     *
     * frequency =
     * timer_clock /
     * ((ARR + 1) * (PSC + 1))
     *
     * ARR is fixed at 99 so there are 100 duty-cycle steps.
     */

    tim->ARR = 100U - 1U;
     
    uint32_t timer_clock_hz = uses_apb2 ? APB2ClockRateHz : APB1ClockRateHz;
    uint32_t denominator = frequency_hz * (tim->ARR + 1U);

    if (denominator == 0U || denominator > timer_clock_hz) {
        return false;
    }

    tim->PSC = (timer_clock_hz / denominator) - 1U;

    switch (channels_en) {
        case 4U:
            tim->CCR4 = 0U;

            tim->CCMR2 &= ~TIM_CCMR2_OC4M_Msk;
            tim->CCMR |= TIM_CCMR2_OC4M_2 | TIM_CCMR1_OC4M_1;

            tim->CCMR2 |= TIM_CCMR2_OC4PE;
            tim->CCER |= TIM_CCER_CC4E;
        
        case 3U:
            tim->CCR3 = 0U;

            tim->CCMR2 &= ~TIM_CCMR2_OC3M_Msk;
            tim->CCMR |= TIM_CCMR2_OC3M_2 | TIM_CCMR1_OC3M_1;

            tim->CCMR2 |= TIM_CCMR2_OC3PE;
            tim->CCER |= TIM_CCER_CC3E;   
            
        case 2U:
            tim->CCR2 = 0U;

            tim->CCMR2 &= ~TIM_CCMR2_OC2M_Msk;
            tim->CCMR |= TIM_CCMR2_OC2M_2 | TIM_CCMR1_OC2M_1;

            tim->CCMR2 |= TIM_CCMR2_OC2PE;
            tim->CCER |= TIM_CCER_CC2E;       
        
        case 1U:
            tim->CCR1 = 0U;

            tim->CCMR1 &= ~TIM_CCMR1_OC1M_Msk;
            tim->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

            tim->CCMR1 |= TIM_CCMR1_OC1PE;
            tim->CCER |= TIM_CCER_CC1E;
            break;

        default:
            return false;
    }

    if (requires_main_out_en) {
        tim->BDTR |= TIm_BDTR_MOE;
    }

    tim->CR1 &= ~TIM_CR1_DIR;
    tim->CR1 |= TIM_CR1_ARPE;

    tim->CNT = 0U;
    tim->EGR |= TIM_EGR_UG;

    tim->CR1 |= TIM_CR1_CEN;

    return true;
}

void PHAL_PWMsetPercent(TIM_TypeDef *tim, uint8_t channel, uint8_t percent) {
    if (tim == NULL) {
        return;
    }

    if (percent > 100U) {
        percent = 100U;
    }

    uint32_t compare_value = ((tim->ARR + 1U) * percent) / 100U;

    switch (channel) {
        case 1U:
            tim->CCR1 = compare_value;
            break;

        case 2U:
            tim->CCR2 = compare_value;
            break;  

        case 3U:
            tim->CCR3 = compare_value;
            break;

        case 4U:
            tim->CCR4 = compare_value;
            break;

        default:
            break;
    }
}