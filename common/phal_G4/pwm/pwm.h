/**
 * @file pwm.h
 * @author Natasha Pandit (npandit@purdue.edu)
 * @brief PWM driver for STM32G4
 * @date 2026-07-26
 */

#ifndef _PHAL_PWM_H
#define _PHAL_PWM_H

#include "common/phal_G4/phal_G4.h"

#include <stdbool.h>
#include <stdint.h>

// initialize PWM outputs on timer
bool PHAL_initPWM(TIM_TypeDef* tim, uint32_t frequency_hz, uint8_t channels_en);

// set PWM duty cycle for timer channel
void PHAL_PWMsetPercent(TIM_TypeDef* tim, uint8_t channel, uint8_t percent);

#endif
