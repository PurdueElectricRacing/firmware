#include "fan_control.h"
#include "common/phal_F4_F7/pwm/pwm.h"

bool fanControlInit() {
    return PHAL_initPWM(PWM_FREQUENCY_HZ, FAN_PWM_TIM, 2);
}

void setFan1Speed(uint8_t fan_speed) {
    PHAL_PWMsetPercent(FAN_PWM_TIM, 1, fan_speed);
}

void setFan2Speed(uint8_t fan_speed) {
    PHAL_PWMsetPercent(FAN_PWM_TIM, 2, fan_speed);
}