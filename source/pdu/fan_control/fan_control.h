#ifndef _FAN_CONTROL_H_
#define _FAN_CONTROL_H_

#include <stdbool.h>
#include "stm32f407xx.h"
#include "main.h"

#define PWM_FREQUENCY_HZ (25000) // PWM frequency to be 25kHz
#define FAN_PWM_TIM (FAN_1_PWM_TIM) // Fan 1 and 2 user same timer
//
bool fanControlInit();
void setFanSpeed(uint8_t fan_speed);
uint32_t getFan1Speed();

#endif // _FAN_CONTROL_H_
