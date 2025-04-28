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
#include "led.h"
#include "fan_control.h"

volatile cooling_request_t cr;
static void calculate_cooling_periodic();

void coolingInit() {
    cr.fan1_speed = 0;
    cr.fan2_speed = 0;
    cr.fan1_status = false;
    cr.fan2_status = false;
    cr.pump1_status = false;
    cr.pump2_status = false;
    cr.aux_status = false;
}

void update_cooling_periodic() {
    // Update switch statuses based off values in cr struct
    calculate_cooling_periodic();

    setSwitch(SW_FAN_1, cr.fan1_status);
    setSwitch(SW_FAN_2, cr.fan2_status);
    setSwitch(SW_PUMP_1, cr.pump1_status);
    setSwitch(SW_PUMP_2, cr.pump2_status);
    setSwitch(SW_AUX, cr.aux_status);

    // Set fan speeds based off of fan speeds in cr struct
    if (cr.fan1_status) setFan1Speed(cr.fan1_speed);
    if (cr.fan2_status) setFan2Speed(cr.fan2_speed);

    SEND_COOLANT_OUT(cr.fan1_speed, cr.fan2_speed,
                     cr.pump2_status, cr.aux_status, cr.pump1_status);
}

static void calculate_cooling_periodic()
{
    // GPS not stale implies GPS fix
    bool car_moving = !can_data.gps_speed.stale && can_data.gps_speed.gps_speed <= GPS_SPEED_MOVING;

    // PUMP1: Battery pumps
    // Enable if above 25C
    // TODO handle stale
    if (!can_data.max_cell_temp.stale &&
        (can_data.max_cell_temp.max_temp >= BATT_COOLING_ENABLE_TEMP * 10))
    {
        cr.pump1_status = true;
    }
    else
    {
        cr.pump1_status = false;
    }

    // FAN1: Battery fans
    // Enable battery fans only when car stopped since there is little airflow
    if (car_moving)
    {
        cr.fan1_status = true;
        if (!cr.fan1_speed) // Don't override dash request values
        cr.fan1_speed = 100; // Default to 100% duty
    }
    else
    {
        cr.fan1_status = false;
        cr.fan1_speed = 0;
    }

    // PUMP2: Motor pumps
    // Enable motor pumps if above 60C
    if (!can_data.rear_motor_temps.stale &&
        ((can_data.rear_motor_temps.left_mot_temp >= MOTOR_COOLING_ENABLE_TEMP ||
          can_data.rear_motor_temps.right_mot_temp >= MOTOR_COOLING_ENABLE_TEMP)))
    {
        cr.pump2_status = true;
    }
    else
    {
        cr.pump2_status = false;
    }

    // FAN2: Motor fans
    // Enable motor fans if (above 100C) || (car stopped && temp > 60)
    if (!can_data.rear_motor_temps.stale &&
        ((can_data.rear_motor_temps.left_mot_temp >= MOTOR_COOLING_MAX_TEMP ||
         can_data.rear_motor_temps.right_mot_temp >= MOTOR_COOLING_MAX_TEMP) ||
        ((can_data.rear_motor_temps.left_mot_temp >= MOTOR_COOLING_ENABLE_TEMP ||
         can_data.rear_motor_temps.right_mot_temp >= MOTOR_COOLING_ENABLE_TEMP) &&
         car_moving)))
    {
        cr.fan2_status = true;
        if (!cr.fan2_speed) // Don't override dash request values
        cr.fan2_speed = 100;
    }
    else
    {
        cr.fan2_status = false;
        cr.fan2_speed = 0;
    }
}

void cooling_driver_request_CALLBACK(CanParsedData_t *msg_data_a)
{
    // Only receive fan speed values from dash now
    cr.fan1_speed = CLAMP(can_data.cooling_driver_request.batt_fan, 0, 100);
    cr.fan2_speed = CLAMP(can_data.cooling_driver_request.dt_fan, 0, 100);
    //cr.pump1_status = can_data.cooling_driver_request.dt_pump; // determine if pumps are on or off
    //cr.pump2_status = can_data.cooling_driver_request.batt_pump;
    //cr.aux_status = can_data.cooling_driver_request.batt_pump2;
}
