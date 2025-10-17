/**
 * @file pwm.h
 * @author Seth Baird (baird33@purdue.edu)
 * @date 2025-10-02
 */

#ifndef _PHAL_PWM_H
#define _PHAL_PWM_H

#include "common/phal_F4_F7/phal_F4_F7.h"
#include "common/phal/gpio.h"

bool PHAL_initPWM(uint32_t frequency_hz, TIM_TypeDef* tim, uint8_t channelsToEnable);
void PHAL_PWMsetPercent(TIM_TypeDef* tim, uint8_t channel, uint8_t percent);

#endif