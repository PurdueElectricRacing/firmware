/**
 * @file cooling_callbacks.c
 * @brief callback functions for bang-bang control
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "cooling_callbacks.h"

#include "switches.h"

void powertrain_pumps_on(void) {
    switches_set_state(SW_PUMP_1, true);
    switches_set_state(SW_PUMP_2, true);
}

void powertrain_pumps_off(void) {
    switches_set_state(SW_PUMP_1, false);
    switches_set_state(SW_PUMP_2, false);
}

void hx_fan_on(void) {
    switches_set_state(SW_HXFAN, true);
}

void hx_fan_off(void) {
    switches_set_state(SW_HXFAN, false);
}

void battery_fans_on(void) {
    switches_set_state(SW_FAN_1, true);
    switches_set_state(SW_FAN_2, true);
    switches_set_state(SW_FAN_3, true);
    switches_set_state(SW_FAN_4, true);
}

void battery_fans_off(void) {
    switches_set_state(SW_FAN_1, false);
    switches_set_state(SW_FAN_2, false);
    switches_set_state(SW_FAN_3, false);
    switches_set_state(SW_FAN_4, false);
}