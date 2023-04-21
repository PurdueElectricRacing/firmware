/**
 * @file can_parse.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief Parsing of CAN messages using auto-generated structures with bit-fields
 * @version 0.1
 * @date 2021-09-15
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef _CAN_PARSE_H_
#define _CAN_PARSE_H_

#include "common/queue/queue.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "Main_Module"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_MAIN_HB 0x4001901
#define ID_TORQUE_REQUEST_MAIN 0x4000041
#define ID_FLOWRATE_TEMPS 0x4000881
#define ID_COOLANT_OUT 0x40008c1
#define ID_GEARBOX 0x10000901
#define ID_LWS_CONFIG 0x7c0
#define ID_VOLTAGE_RAILS 0x10001901
#define ID_PRECHARGE_STATE 0x8001881
#define ID_CURRENT_MEAS 0x10001941
#define ID_MCU_STATUS 0x10001981
#define ID_REAR_MC_STATUS 0x4001941
#define ID_REAR_MOTOR_CURRENTS_TEMPS 0xc0002c1
#define ID_REAR_CONTROLLER_TEMPS 0xc000301
#define ID_REAR_WHEEL_SPEEDS 0x8000381
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_DAQ_RESPONSE_MAIN_MODULE 0x17ffffc1
#define ID_RAW_THROTTLE_BRAKE 0x14000285
#define ID_FILT_THROTTLE_BRAKE 0x4000245
#define ID_START_BUTTON 0x4000005
#define ID_DASHBOARD_HB 0x4001905
#define ID_MAX_CELL_TEMP 0x404e604
#define ID_LWS_STANDARD 0x2b0
#define ID_MAIN_MODULE_BL_CMD 0x409c43e
#define ID_COOLING_DRIVER_REQUEST 0xc0002c5
#define ID_FAULT_SYNC_DRIVELINE 0x8ca83
#define ID_FAULT_SYNC_DASHBOARD 0x8cb05
#define ID_FAULT_SYNC_PRECHARGE 0x8cac4
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8ca42
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_MAIN_MODULE 0x14000072
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAIN_HB 2
#define DLC_TORQUE_REQUEST_MAIN 8
#define DLC_FLOWRATE_TEMPS 8
#define DLC_COOLANT_OUT 3
#define DLC_GEARBOX 2
#define DLC_LWS_CONFIG 2
#define DLC_VOLTAGE_RAILS 8
#define DLC_PRECHARGE_STATE 4
#define DLC_CURRENT_MEAS 7
#define DLC_MCU_STATUS 5
#define DLC_REAR_MC_STATUS 6
#define DLC_REAR_MOTOR_CURRENTS_TEMPS 8
#define DLC_REAR_CONTROLLER_TEMPS 2
#define DLC_REAR_WHEEL_SPEEDS 8
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_DAQ_RESPONSE_MAIN_MODULE 8
#define DLC_RAW_THROTTLE_BRAKE 8
#define DLC_FILT_THROTTLE_BRAKE 3
#define DLC_START_BUTTON 1
#define DLC_DASHBOARD_HB 1
#define DLC_MAX_CELL_TEMP 2
#define DLC_LWS_STANDARD 5
#define DLC_MAIN_MODULE_BL_CMD 5
#define DLC_COOLING_DRIVER_REQUEST 5
#define DLC_FAULT_SYNC_DRIVELINE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_PRECHARGE 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_MAIN_MODULE 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAIN_HB(queue, car_state_, precharge_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_HB, .DLC=DLC_MAIN_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_hb.car_state = car_state_;\
        data_a->main_hb.precharge_state = precharge_state_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TORQUE_REQUEST_MAIN(queue, front_left_, front_right_, rear_left_, rear_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TORQUE_REQUEST_MAIN, .DLC=DLC_TORQUE_REQUEST_MAIN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->torque_request_main.front_left = front_left_;\
        data_a->torque_request_main.front_right = front_right_;\
        data_a->torque_request_main.rear_left = rear_left_;\
        data_a->torque_request_main.rear_right = rear_right_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FLOWRATE_TEMPS(queue, battery_in_temp_, battery_out_temp_, drivetrain_in_temp_, drivetrain_out_temp_, battery_flowrate_, drivetrain_flowrate_, battery_fan_speed_, drivetrain_fan_speed_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FLOWRATE_TEMPS, .DLC=DLC_FLOWRATE_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->flowrate_temps.battery_in_temp = battery_in_temp_;\
        data_a->flowrate_temps.battery_out_temp = battery_out_temp_;\
        data_a->flowrate_temps.drivetrain_in_temp = drivetrain_in_temp_;\
        data_a->flowrate_temps.drivetrain_out_temp = drivetrain_out_temp_;\
        data_a->flowrate_temps.battery_flowrate = battery_flowrate_;\
        data_a->flowrate_temps.drivetrain_flowrate = drivetrain_flowrate_;\
        data_a->flowrate_temps.battery_fan_speed = battery_fan_speed_;\
        data_a->flowrate_temps.drivetrain_fan_speed = drivetrain_fan_speed_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_COOLANT_OUT(queue, bat_fan_, dt_fan_, bat_pump_, bat_pump_aux_, dt_pump_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_COOLANT_OUT, .DLC=DLC_COOLANT_OUT, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->coolant_out.bat_fan = bat_fan_;\
        data_a->coolant_out.dt_fan = dt_fan_;\
        data_a->coolant_out.bat_pump = bat_pump_;\
        data_a->coolant_out.bat_pump_aux = bat_pump_aux_;\
        data_a->coolant_out.dt_pump = dt_pump_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_GEARBOX(queue, l_temp_, r_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_GEARBOX, .DLC=DLC_GEARBOX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->gearbox.l_temp = l_temp_;\
        data_a->gearbox.r_temp = r_temp_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_LWS_CONFIG(queue, CCW_, Reserved_1_, Reserved_2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_LWS_CONFIG, .DLC=DLC_LWS_CONFIG, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->LWS_Config.CCW = CCW_;\
        data_a->LWS_Config.Reserved_1 = Reserved_1_;\
        data_a->LWS_Config.Reserved_2 = Reserved_2_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTAGE_RAILS(queue, LV24_, LV12_, LV5_, LV3V3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTAGE_RAILS, .DLC=DLC_VOLTAGE_RAILS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->voltage_rails.LV24 = LV24_;\
        data_a->voltage_rails.LV12 = LV12_;\
        data_a->voltage_rails.LV5 = LV5_;\
        data_a->voltage_rails.LV3V3 = LV3V3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PRECHARGE_STATE(queue, v_mc_, v_bat_, precharge_complete_, precharge_error_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PRECHARGE_STATE, .DLC=DLC_PRECHARGE_STATE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->precharge_state.v_mc = v_mc_;\
        data_a->precharge_state.v_bat = v_bat_;\
        data_a->precharge_state.precharge_complete = precharge_complete_;\
        data_a->precharge_state.precharge_error = precharge_error_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CURRENT_MEAS(queue, LV24_I_, LV5_I_, MCU_TEMP_, LV3V3_PG_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CURRENT_MEAS, .DLC=DLC_CURRENT_MEAS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->current_meas.LV24_I = LV24_I_;\
        data_a->current_meas.LV5_I = LV5_I_;\
        data_a->current_meas.MCU_TEMP = MCU_TEMP_;\
        data_a->current_meas.LV3V3_PG = LV3V3_PG_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_MCU_STATUS(queue, sched_skips_, foreground_use_, background_use_, sched_error_, can_tx_fails_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MCU_STATUS, .DLC=DLC_MCU_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mcu_status.sched_skips = sched_skips_;\
        data_a->mcu_status.foreground_use = foreground_use_;\
        data_a->mcu_status.background_use = background_use_;\
        data_a->mcu_status.sched_error = sched_error_;\
        data_a->mcu_status.can_tx_fails = can_tx_fails_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_MC_STATUS(queue, rear_left_motor_, rear_left_motor_link_, rear_left_last_link_error_, rear_right_motor_, rear_right_motor_link_, rear_right_last_link_error_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MC_STATUS, .DLC=DLC_REAR_MC_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_mc_status.rear_left_motor = rear_left_motor_;\
        data_a->rear_mc_status.rear_left_motor_link = rear_left_motor_link_;\
        data_a->rear_mc_status.rear_left_last_link_error = rear_left_last_link_error_;\
        data_a->rear_mc_status.rear_right_motor = rear_right_motor_;\
        data_a->rear_mc_status.rear_right_motor_link = rear_right_motor_link_;\
        data_a->rear_mc_status.rear_right_last_link_error = rear_right_last_link_error_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_MOTOR_CURRENTS_TEMPS(queue, left_current_, right_current_, left_temp_, right_temp_, right_voltage_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_CURRENTS_TEMPS, .DLC=DLC_REAR_MOTOR_CURRENTS_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_currents_temps.left_current = left_current_;\
        data_a->rear_motor_currents_temps.right_current = right_current_;\
        data_a->rear_motor_currents_temps.left_temp = left_temp_;\
        data_a->rear_motor_currents_temps.right_temp = right_temp_;\
        data_a->rear_motor_currents_temps.right_voltage = right_voltage_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_CONTROLLER_TEMPS(queue, left_temp_, right_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_CONTROLLER_TEMPS, .DLC=DLC_REAR_CONTROLLER_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_controller_temps.left_temp = left_temp_;\
        data_a->rear_controller_temps.right_temp = right_temp_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_REAR_WHEEL_SPEEDS(queue, left_speed_mc_, right_speed_mc_, left_speed_sensor_, right_speed_sensor_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_WHEEL_SPEEDS, .DLC=DLC_REAR_WHEEL_SPEEDS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_wheel_speeds.left_speed_mc = left_speed_mc_;\
        data_a->rear_wheel_speeds.right_speed_mc = right_speed_mc_;\
        data_a->rear_wheel_speeds.left_speed_sensor = left_speed_sensor_;\
        data_a->rear_wheel_speeds.right_speed_sensor = right_speed_sensor_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FAULT_SYNC_MAIN_MODULE(queue, idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_MAIN_MODULE, .DLC=DLC_FAULT_SYNC_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_main_module.idx = idx_;\
        data_a->fault_sync_main_module.latched = latched_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_MAIN_MODULE(queue, daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_MAIN_MODULE, .DLC=DLC_DAQ_RESPONSE_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_MAIN_MODULE.daq_response = daq_response_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_RAW_THROTTLE_BRAKE 15
#define UP_FILT_THROTTLE_BRAKE 15
#define UP_DASHBOARD_HB 100
#define UP_MAX_CELL_TEMP 500
#define UP_LWS_STANDARD 15
#define UP_COOLING_DRIVER_REQUEST 5
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_IDLE,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_FATAL,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
    CAR_STATE_FAN_CTRL,
} car_state_t;

typedef enum {
    REAR_LEFT_MOTOR_DISCONNECTED,
    REAR_LEFT_MOTOR_CONNECTED,
    REAR_LEFT_MOTOR_CONFIG,
    REAR_LEFT_MOTOR_ERROR,
} rear_left_motor_t;

typedef enum {
    REAR_LEFT_MOTOR_LINK_DISCONNECTED,
    REAR_LEFT_MOTOR_LINK_ATTEMPTING,
    REAR_LEFT_MOTOR_LINK_VERIFYING,
    REAR_LEFT_MOTOR_LINK_DELAY,
    REAR_LEFT_MOTOR_LINK_CONNECTED,
    REAR_LEFT_MOTOR_LINK_FAIL,
} rear_left_motor_link_t;

typedef enum {
    REAR_LEFT_LAST_LINK_ERROR_NONE,
    REAR_LEFT_LAST_LINK_ERROR_NOT_SERIAL,
    REAR_LEFT_LAST_LINK_ERROR_CMD_TIMEOUT,
    REAR_LEFT_LAST_LINK_ERROR_GEN_TIMEOUT,
} rear_left_last_link_error_t;

typedef enum {
    REAR_RIGHT_MOTOR_DISCONNECTED,
    REAR_RIGHT_MOTOR_CONNECTED,
    REAR_RIGHT_MOTOR_CONFIG,
    REAR_RIGHT_MOTOR_ERROR,
} rear_right_motor_t;

typedef enum {
    REAR_RIGHT_MOTOR_LINK_DISCONNECTED,
    REAR_RIGHT_MOTOR_LINK_ATTEMPTING,
    REAR_RIGHT_MOTOR_LINK_VERIFYING,
    REAR_RIGHT_MOTOR_LINK_DELAY,
    REAR_RIGHT_MOTOR_LINK_CONNECTED,
    REAR_RIGHT_MOTOR_LINK_FAIL,
} rear_right_motor_link_t;

typedef enum {
    REAR_RIGHT_LAST_LINK_ERROR_NONE,
    REAR_RIGHT_LAST_LINK_ERROR_NOT_SERIAL,
    REAR_RIGHT_LAST_LINK_ERROR_CMD_TIMEOUT,
    REAR_RIGHT_LAST_LINK_ERROR_GEN_TIMEOUT,
} rear_right_last_link_error_t;

/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t car_state: 8;
        uint64_t precharge_state: 1;
    } main_hb;
    struct {
        uint64_t front_left: 16;
        uint64_t front_right: 16;
        uint64_t rear_left: 16;
        uint64_t rear_right: 16;
    } torque_request_main;
    struct {
        uint64_t battery_in_temp: 8;
        uint64_t battery_out_temp: 8;
        uint64_t drivetrain_in_temp: 8;
        uint64_t drivetrain_out_temp: 8;
        uint64_t battery_flowrate: 8;
        uint64_t drivetrain_flowrate: 8;
        uint64_t battery_fan_speed: 8;
        uint64_t drivetrain_fan_speed: 8;
    } flowrate_temps;
    struct {
        uint64_t bat_fan: 8;
        uint64_t dt_fan: 8;
        uint64_t bat_pump: 1;
        uint64_t bat_pump_aux: 1;
        uint64_t dt_pump: 1;
    } coolant_out;
    struct {
        uint64_t l_temp: 8;
        uint64_t r_temp: 8;
    } gearbox;
    struct {
        uint64_t CCW: 3;
        uint64_t Reserved_1: 5;
        uint64_t Reserved_2: 8;
    } LWS_Config;
    struct {
        uint64_t LV24: 16;
        uint64_t LV12: 16;
        uint64_t LV5: 16;
        uint64_t LV3V3: 16;
    } voltage_rails;
    struct {
        uint64_t v_mc: 12;
        uint64_t v_bat: 12;
        uint64_t precharge_complete: 1;
        uint64_t precharge_error: 1;
    } precharge_state;
    struct {
        uint64_t LV24_I: 16;
        uint64_t LV5_I: 16;
        uint64_t MCU_TEMP: 16;
        uint64_t LV3V3_PG: 1;
    } current_meas;
    struct {
        uint64_t sched_skips: 8;
        uint64_t foreground_use: 8;
        uint64_t background_use: 8;
        uint64_t sched_error: 8;
        uint64_t can_tx_fails: 8;
    } mcu_status;
    struct {
        uint64_t rear_left_motor: 8;
        uint64_t rear_left_motor_link: 8;
        uint64_t rear_left_last_link_error: 8;
        uint64_t rear_right_motor: 8;
        uint64_t rear_right_motor_link: 8;
        uint64_t rear_right_last_link_error: 8;
    } rear_mc_status;
    struct {
        uint64_t left_current: 16;
        uint64_t right_current: 16;
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
        uint64_t right_voltage: 16;
    } rear_motor_currents_temps;
    struct {
        uint64_t left_temp: 8;
        uint64_t right_temp: 8;
    } rear_controller_temps;
    struct {
        uint64_t left_speed_mc: 16;
        uint64_t right_speed_mc: 16;
        uint64_t left_speed_sensor: 16;
        uint64_t right_speed_sensor: 16;
    } rear_wheel_speeds;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
    struct {
        uint64_t daq_response: 64;
    } daq_response_MAIN_MODULE;
    struct {
        uint64_t throttle: 12;
        uint64_t throttle_right: 12;
        uint64_t brake: 12;
        uint64_t brake_right: 12;
        uint64_t brake_pot: 12;
    } raw_throttle_brake;
    struct {
        uint64_t throttle: 12;
        uint64_t brake: 12;
    } filt_throttle_brake;
    struct {
        uint64_t start: 1;
    } start_button;
    struct {
        uint64_t apps_faulted: 1;
        uint64_t bse_faulted: 1;
        uint64_t apps_brake_faulted: 1;
    } dashboard_hb;
    struct {
        uint64_t max_temp: 16;
    } max_cell_temp;
    struct {
        uint64_t LWS_ANGLE: 16;
        uint64_t LWS_SPEED: 8;
        uint64_t Ok: 1;
        uint64_t Cal: 1;
        uint64_t Trim: 1;
        uint64_t Reserved_1: 5;
        uint64_t Reserved_2: 8;
    } LWS_Standard;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } main_module_bl_cmd;
    struct {
        uint64_t dt_pump: 8;
        uint64_t dt_fan: 8;
        uint64_t batt_pump: 8;
        uint64_t batt_pump2: 8;
        uint64_t batt_fan: 8;
    } cooling_driver_request;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_driveline;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_dashboard;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_precharge;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_torque_vector;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_test_node;
    struct {
        uint64_t id: 16;
        uint64_t value: 1;
    } set_fault;
    struct {
        uint64_t id: 16;
    } return_fault_control;
    struct {
        uint64_t daq_command: 64;
    } daq_command_MAIN_MODULE;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t throttle;
        uint16_t throttle_right;
        uint16_t brake;
        uint16_t brake_right;
        uint16_t brake_pot;
        uint8_t stale;
        uint32_t last_rx;
    } raw_throttle_brake;
    struct {
        uint16_t throttle;
        uint16_t brake;
        uint8_t stale;
        uint32_t last_rx;
    } filt_throttle_brake;
    struct {
        uint8_t start;
    } start_button;
    struct {
        uint8_t apps_faulted;
        uint8_t bse_faulted;
        uint8_t apps_brake_faulted;
        uint8_t stale;
        uint32_t last_rx;
    } dashboard_hb;
    struct {
        uint16_t max_temp;
        uint8_t stale;
        uint32_t last_rx;
    } max_cell_temp;
    struct {
        int16_t LWS_ANGLE;
        uint8_t LWS_SPEED;
        uint8_t Ok;
        uint8_t Cal;
        uint8_t Trim;
        uint8_t Reserved_1;
        uint8_t Reserved_2;
        uint8_t stale;
        uint32_t last_rx;
    } LWS_Standard;
    struct {
        uint8_t cmd;
        uint32_t data;
    } main_module_bl_cmd;
    struct {
        uint8_t dt_pump;
        uint8_t dt_fan;
        uint8_t batt_pump;
        uint8_t batt_pump2;
        uint8_t batt_fan;
        uint8_t stale;
        uint32_t last_rx;
    } cooling_driver_request;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_driveline;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_dashboard;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_precharge;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_torque_vector;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_test_node;
    struct {
        uint16_t id;
        uint8_t value;
    } set_fault;
    struct {
        uint16_t id;
    } return_fault_control;
    struct {
        uint64_t daq_command;
    } daq_command_MAIN_MODULE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_MAIN_MODULE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void main_module_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void cooling_driver_request_CALLBACK(CanParsedData_t* msg_data_a);
extern void handleCallbacks(uint16_t id, bool latched);
extern void set_fault_daq(uint16_t id, bool value);
extern void return_fault_control(uint16_t id);
extern void send_fault(uint16_t id, bool latched);
/* END AUTO EXTERN CALLBACK */

/* BEGIN AUTO EXTERN RX IRQ */
/* END AUTO EXTERN RX IRQ */

/**
 * @brief Setup queue and message filtering
 *
 * @param q_rx_can RX buffer of CAN messages
 */
void initCANParse(q_handle_t* q_rx_can_a);

/**
 * @brief Pull message off of rx buffer,
 *        update can_data struct,
 *        check for stale messages
 */
void canRxUpdate();

/**
 * @brief Process any rx message callbacks from the CAN Rx IRQ
 *
 * @param rx rx data from message just recieved
 */
void canProcessRxIRQs(CanMsgTypeDef_t* rx);

#endif