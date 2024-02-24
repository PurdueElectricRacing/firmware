#include "cooling.h"
#include "auto_switch.h"
#include "led.h"
#include "fan_control.h"

volatile cooling_request_t cooling_request;

void update_cooling_periodic() {
    setSwitch(SW_PUMP_1, cooling_request.pump1_status);
    setSwitch(SW_PUMP_2, cooling_request.pump2_status);
    setSwitch(SW_AUX, cooling_request.aux_status);
    //if (can_data.cooling_driver_request.batt_fan > 0) {
    setSwitch(SW_FAN_1, cooling_request.fan1_status);
    
    
    //if (can_data.cooling_driver_request.dt_fan > 0) {
    setSwitch(SW_FAN_2, cooling_request.fan2_status);
    //}
    
    setFan1Speed(cooling_request.fan1_speed);
    setFan2Speed(cooling_request.fan2_speed);
}
void cooling_driver_request_CALLBACK(CanParsedData_t *msg_data_a)
{
    cooling_request.fan1_status = can_data.cooling_driver_request.dt_fan > 0 ? true : false;
    cooling_request.fan2_status = can_data.cooling_driver_request.batt_fan > 0 ? true : false;
    cooling_request.fan1_speed = can_data.cooling_driver_request.dt_fan > 100 ? 100 : can_data.cooling_driver_request.dt_fan;
    cooling_request.fan2_speed = can_data.cooling_driver_request.batt_fan > 100 ? 100 : can_data.cooling_driver_request.batt_fan;
    cooling_request.pump1_status = can_data.cooling_driver_request.dt_pump;
    cooling_request.pump2_status = can_data.cooling_driver_request.batt_pump;
    cooling_request.aux_status = can_data.cooling_driver_request.batt_pump2;
}

