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

bool PHAL_initPWM(TIM_TypeDef* tim, uint32_t frequency_hz, uint8_t channels_en);
void PHAL_PWMsetPercent(TIM_TypeDef* tim, uint8_t channel, uint8_t percent);

#endif
