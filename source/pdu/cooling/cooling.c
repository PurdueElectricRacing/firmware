/**
 * @file cooling.c
 * @author Nicolas Vera (nverapae@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2024-2-29
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "cooling.h"

#include "auto_switch.h"
#include "common/bangbang/bangbang.h"
#include "fan_control.h"
#include "led.h"

volatile cooling_request_t cr;
static void calculate_cooling_periodic();

static void set_pump1_on();
static void set_pump1_off();
static void set_fan1_on();
static void set_fan1_off();
static void set_pump2_on();
static void set_pump2_off();
static void set_fan2_on();
static void set_fan2_off();

static bool batt_fan_autospeed;
static bool motor_fan_autospeed;

INIT_BANG_BANG(batt_pump_controller,
               BATT_PUMP_ON_TEMP_C,
               BATT_PUMP_OFF_TEMP_C,
               set_pump1_on,
               set_pump1_off,
               BANGBANG_MIN_SWITCH_MS)

INIT_BANG_BANG(batt_fan_controller,
               BATT_FAN_ON_TEMP_C,
               BATT_FAN_OFF_TEMP_C,
               set_fan1_on,
               set_fan1_off,
               BANGBANG_MIN_SWITCH_MS)

INIT_BANG_BANG(motor_pump_controller,
               MOTOR_PUMP_ON_TEMP_C,
               MOTOR_PUMP_OFF_TEMP_C,
               set_pump2_on,
               set_pump2_off,
               BANGBANG_MIN_SWITCH_MS)

INIT_BANG_BANG(motor_fan_controller,
               MOTOR_FAN_ON_TEMP_C,
               MOTOR_FAN_OFF_TEMP_C,
               set_fan2_on,
               set_fan2_off,
               BANGBANG_MIN_SWITCH_MS)

void coolingInit() {
    cr.fan1_speed       = 0;
    cr.fan2_speed       = 0;
    cr.fan3_speed       = 0;
    cr.fan4_speed       = 0;
    cr.fan1_status      = false;
    cr.fan2_status      = false;
    cr.fan3_status      = false;
    cr.fan4_status      = false;
    cr.pump1_status     = false;
    cr.pump2_status     = false;
    cr.hxfan_status     = false;
    batt_fan_autospeed  = true;
    motor_fan_autospeed = true;
}

void update_cooling_periodic() {
    // Update switch statuses based off values in cr struct
    calculate_cooling_periodic();

    setSwitch(SW_FAN_1, cr.fan1_status);
    setSwitch(SW_FAN_2, cr.fan2_status);
    setSwitch(SW_FAN_3, cr.fan3_status);
    setSwitch(SW_FAN_4, cr.fan4_status);
    setSwitch(SW_PUMP_1, cr.pump1_status);
    setSwitch(SW_PUMP_2, cr.pump2_status);
    setSwitch(SW_HXFAN, cr.hxfan_status);

    // Set fan speeds based off of fan speeds in cr struct
    if (cr.fan1_status)
        setFan1Speed(cr.fan1_speed);
    else
        setFan1Speed(0);
    if (cr.fan2_status)
        setFan2Speed(cr.fan2_speed);
    else
        setFan2Speed(0);
    if (cr.fan3_status)
        setFan3Speed(cr.fan3_speed);
    else
        setFan3Speed(0);
    if (cr.fan4_status)
        setFan4Speed(cr.fan4_speed);
    else
        setFan4Speed(0);

    CAN_SEND_coolant_out(cr.fan1_speed,
                         cr.fan2_speed,
                         cr.pump2_status,
                         cr.hxfan_status,
                         cr.pump1_status);
}

static void calculate_cooling_periodic() {
    // GPS not stale implies GPS fix
    bool not_moving = !can_data.gps_speed.stale && can_data.gps_speed.gps_speed <= GPS_SPEED_MOVING;

    uint32_t now_ms = sched.os_ticks;

    // PUMP1: Battery pumps
    // Enable if above 30C
    // TODO handle stale
    if (can_data.max_cell_temp.stale) {
        batt_pump_controller.last_switch_ms = now_ms;
        set_pump1_off();
    } else {
        bangbang_update(&batt_pump_controller, can_data.max_cell_temp.max_temp / 10.0f, now_ms);
    }

    // FAN1: Battery fans
    // Enable battery fans only when car stopped since there is little airflow
    if (can_data.max_cell_temp.stale || !not_moving) {
        batt_fan_controller.last_switch_ms = now_ms;
        set_fan1_off();
    } else {
        bangbang_update(&batt_fan_controller, can_data.max_cell_temp.max_temp / 10.0f, now_ms);
    }

    // PUMP2: Motor pumps
    // Enable motor pumps if above 60C
    if (can_data.rear_motor_temps.stale) {
        motor_pump_controller.last_switch_ms = now_ms;
        set_pump2_off();
    } else {
        float motor_temp = (float)MAX(can_data.rear_motor_temps.left_mot_temp,
                                      can_data.rear_motor_temps.right_mot_temp);
        bangbang_update(&motor_pump_controller, motor_temp, now_ms);
    }

    // FAN2: Motor fans
    // Enable motor fans if (above 100C) || (car stopped && temp > 60)
    if (can_data.rear_motor_temps.stale) {
        motor_fan_controller.last_switch_ms = now_ms;
        set_fan2_off();
    } else {
        float motor_temp = (float)MAX(can_data.rear_motor_temps.left_mot_temp,
                                      can_data.rear_motor_temps.right_mot_temp);
        if (not_moving) {
            bangbang_update(&motor_fan_controller, motor_temp, now_ms);
        } else if (motor_temp < MOTOR_COOLING_ENABLE_TEMP) {
            motor_fan_controller.last_switch_ms = now_ms;
            set_fan2_off();
        } else {
            bangbang_update(&motor_fan_controller, motor_temp, now_ms);
        }
    }

    cr.fan3_status = cr.fan1_status;
    cr.fan3_speed  = cr.fan1_speed;
    cr.fan4_status = cr.fan2_status;
    cr.fan4_speed  = cr.fan2_speed;
}

static void set_pump1_on() {
    cr.pump1_status = true;
}

static void set_pump1_off() {
    cr.pump1_status = false;
}

static void set_fan1_on() {
    cr.fan1_status = true;
    if (batt_fan_autospeed) {
        cr.fan1_speed = 100;
    }
}

static void set_fan1_off() {
    cr.fan1_status = false;
    if (batt_fan_autospeed) {
        cr.fan1_speed = 0;
    }
}

static void set_pump2_on() {
    cr.pump2_status = true;
}

static void set_pump2_off() {
    cr.pump2_status = false;
}

static void set_fan2_on() {
    cr.fan2_status = true;
    if (motor_fan_autospeed) {
        cr.fan2_speed = 100;
    }
}

static void set_fan2_off() {
    cr.fan2_status = false;
    if (motor_fan_autospeed) {
        cr.fan2_speed = 0;
    }
}

void cooling_driver_request_CALLBACK(can_data_t *p_can_data) {
    // Only receive fan speed values from dash now
    cr.fan1_speed       = CLAMP(can_data.cooling_driver_request.batt_fan, 0, 100);
    cr.fan2_speed       = CLAMP(can_data.cooling_driver_request.dt_fan, 0, 100);
    cr.fan3_speed       = cr.fan1_speed;
    cr.fan4_speed       = cr.fan2_speed;
    batt_fan_autospeed  = cr.fan1_speed == 0;
    motor_fan_autospeed = cr.fan2_speed == 0;
    //cr.pump1_status = can_data.cooling_driver_request.dt_pump; // determine if pumps are on or off
    //cr.pump2_status = can_data.cooling_driver_request.batt_pump;
    //cr.aux_status = can_data.cooling_driver_request.batt_pump2;
}
