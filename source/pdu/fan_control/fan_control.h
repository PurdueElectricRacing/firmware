#ifndef _FAN_CONTROL_H_
#define _FAN_CONTROL_H_

#include "main.h"

#define PWM_FREQUENCY_HZ (25000) // PWM frequency to be 25kHz
#define FAN_PWM_TIM      (FAN_1_PWM_TIM) // Fan 1 and 2 use same timer defined in main.h

bool fanControlInit();
void setFan1Speed(uint8_t fan_speed);
void setFan2Speed(uint8_t fan_speed);

#endif // _FAN_CONTROL_H_
