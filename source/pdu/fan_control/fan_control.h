#ifndef _FAN_CONTROL_H_
#define _FAN_CONTROL_H_

#include <stdbool.h>

#include "main.h"
#include "stm32f407xx.h"

#define PWM_FREQUENCY_HZ (25000) // PWM frequency to be 25kHz
#define FAN_PWM_TIM      (FAN_1_PWM_TIM) // Fan 1 and 2 user same timer

bool fanControlInit();
void setFan1Speed(uint8_t fan_speed);
void setFan2Speed(uint8_t fan_speed);

#endif // _FAN_CONTROL_H_
