/**
 * @file pwm.h
 * @author Natasha Pandit (npandit@purdue.edu)
 * @date 2026-07-26
 */

#ifndef _PHAL_PWM_H
#define _PHAL_PWM_H

#include "common/phal_G4/phal_G4.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PWM_CHANNEL_1 = (1U << 0),
    PWM_CHANNEL_2 = (1U << 1),
    PWM_CHANNEL_3 = (1U << 2),
    PWM_CHANNEL_4 = (1U << 3),
} PWMChannel_t

bool PHAL_initPWM(TIM_TypeDef* tim, uint32_t frequency_hz, uint8_t channels_en);
void PHAL_PWMsetPercent(TIM_TypeDef* tim, uint8_t channel, uint8_t percent);

#endif
