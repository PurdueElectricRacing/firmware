/**
* @file pwm.c
* @brief Public PWM driver interface for STM32G4
* @author Natasha Pandit (npandit@purdue.edu)
*/
#include <stddef.h>

#include "common/phal_G4/pwm/pwm.h"
#include "common/phal_G4/pwm/pwm_priv.h"

bool PHAL_initPWM(TIM_TypeDef* tim, uint32_t frequency_hz, uint8_t channels_en) {
    if (tim == NULL || frequency_hz == 0U) {
        return false;
    }

    PWM_PRIV_TimerInfo_t timer_info;

    if (!PWM_PRIV_getTimerInfo(tim, &timer_info)) {
        return false;
    }

    if (channels_en == 0U || channels_en > timer_info.max_channel) {
        return false;
    }

    /*
     * Keep ARR at 99 to provide 100 duty-cycle steps.
     *
     * frequency = timer_clock / ((ARR + 1) * (PSC + 1))
     */
    const uint32_t auto_reload = 99U;
    const uint32_t period_steps = auto_reload + 1U;

    /*
     * Check for multiplication overflow before calculating
     * frequency_hz * period_steps.
     */
    if (frequency_hz > UINT32_MAX / period_steps) {
        return false;
    }

    const uint32_t denominator = frequency_hz * period_steps;

    if (denominator == 0U || denominator > timer_info.timer_clock_hz) {
        return false;
    }

    const uint32_t divider = timer_info.timer_clock_hz / denominator;

    if (divider == 0U || divider > 65536U) {
        return false;
    }

    const uint32_t prescaler = divider - 1U;

    return PWM_PRIV_initTimer(tim, (uint16_t)prescaler, (uint16_t)auto_reload, channels_en, timer_info.requires_main_out_en);    
}

bool PHAL_PWMsetPercent(TIM_TypeDef *tim, uint8_t channel, uint8_t percent) {
    if (tim == NULL || channel < 1U || channel > 4U || percent > 100U) {
        return false;
    }

    const uint32_t auto_reload = PWM_PRIV_getAutoReload(tim);

    const uint32_t compare_value = ((auto_reload + 1U) * percent) / 100U;

    return PWM_PRIV_setCompare(tim, channel, compare_value);
}