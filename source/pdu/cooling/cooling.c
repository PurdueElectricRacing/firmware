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

volatile cooling_request_t cooling_request;

extern q_handle_t q_tx_can;

void coolingInit() {
    cooling_request.fan1_status = 0;
    cooling_request.fan2_status = 0;
    cooling_request.fan1_speed = 0;
    cooling_request.fan2_speed = 0;
    cooling_request.pump1_status = 0;
    cooling_request.pump2_status = 0;
    cooling_request.aux_status = 0;
}

void update_cooling_periodic() {
    // Update switch statuses based off values in cooling_request struct
    setSwitch(SW_PUMP_1, cooling_request.pump1_status);
    setSwitch(SW_PUMP_2, cooling_request.pump2_status);
    setSwitch(SW_AUX, cooling_request.aux_status);
    setSwitch(SW_FAN_1, cooling_request.fan1_status);
    setSwitch(SW_FAN_2, cooling_request.fan2_status);

    // Set fan speeds based off of fan speeds in cooling_request struct
    setFan1Speed(cooling_request.fan1_speed);
    setFan2Speed(cooling_request.fan2_speed);

    SEND_COOLANT_OUT(q_tx_can, cooling_request.fan2_speed,
        cooling_request.fan1_speed, cooling_request.pump2_status, cooling_request.aux_status,
        cooling_request.pump1_status);
}
void cooling_driver_request_CALLBACK(CanParsedData_t *msg_data_a)
{
    // Set cooling_request struct values to values received in CAN messages accessed through the global can_data variable
    cooling_request.fan2_status = can_data.cooling_driver_request.dt_fan > 0 ? true : false; // determine if fans are on or off
    cooling_request.fan1_status = can_data.cooling_driver_request.batt_fan > 0 ? true : false;
    cooling_request.fan2_speed = can_data.cooling_driver_request.dt_fan > 100 ? 100 : can_data.cooling_driver_request.dt_fan; // cap fan speeds at 100
    cooling_request.fan1_speed = can_data.cooling_driver_request.batt_fan > 100 ? 100 : can_data.cooling_driver_request.batt_fan;
    cooling_request.pump1_status = can_data.cooling_driver_request.dt_pump; // determine if pumps are on or off
    cooling_request.pump2_status = can_data.cooling_driver_request.batt_pump;
    cooling_request.aux_status = can_data.cooling_driver_request.batt_pump2;
}
