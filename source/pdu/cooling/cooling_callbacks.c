#include "main.h"


void motor_pump_off() {

}

void motor_pump_on() {

}

void inverter_pump_off() {

}

void inverter_pump_on() {
    
}

void battery_fans_on() {
    // set the PWMS to full
    PHAL_PWMsetPercent(FAN_PWM_TIM, 1, fan_speed);
    PHAL_PWMsetPercent(FAN_PWM_TIM, 2, fan_speed);
    PHAL_PWMsetPercent(FAN_PWM_TIM, 3, fan_speed);
    PHAL_PWMsetPercent(FAN_PWM_TIM, 4, fan_speed);

    switches_set_state(SW_FAN_1, g_pdu_state.cooling_command.fan1_enabled);
    switches_set_state(SW_FAN_2, g_pdu_state.cooling_command.fan2_enabled);
    switches_set_state(SW_FAN_3, g_pdu_state.cooling_command.fan3_enabled);
    switches_set_state(SW_FAN_4, g_pdu_state.cooling_command.fan4_enabled);
}

void battery_fans_off() {
    // set the PWMS to off
}