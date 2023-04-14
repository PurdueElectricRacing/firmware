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
#define NODE_NAME "Precharge"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_HEAT_REQ 0x8007d2a
#define ID_PACK_CURR 0x4007d6a
#define ID_BALANCE_REQUEST 0xc00002a
#define ID_PRECHARGE_HB 0x4001944
#define ID_ELCON_CHARGER_COMMAND 0x1806e5f4
#define ID_NUM_THEM_BAD 0x80080c4
#define ID_PACK_CHARGE_STATUS 0x8008084
#define ID_GYRO_DATA 0x4008004
#define ID_ACCEL_DATA 0x4008044
#define ID_ANGLE_DATA 0x4008104
#define ID_MAX_CELL_TEMP 0x404e604
#define ID_MOD_CELL_TEMP_AVG 0x14008084
#define ID_MOD_CELL_TEMP_MAX 0x14008104
#define ID_MOD_CELL_TEMP_MIN 0x14008204
#define ID_RAW_CELL_TEMP 0x140080c4
#define ID_FAULT_SYNC_PRECHARGE 0x8cac4
#define ID_DAQ_RESPONSE_PRECHARGE 0x17ffffc4
#define ID_MODULE_TEMP_0 0xbe0
#define ID_MODULE_TEMP_1 0xbe1
#define ID_MODULE_TEMP_2 0xbe2
#define ID_MODULE_TEMP_3 0xbe3
#define ID_MODULE_TEMP_4 0xbe4
#define ID_MODULE_TEMP_5 0xbe5
#define ID_MODULE_TEMP_6 0xbe6
#define ID_MODULE_TEMP_7 0xbe7
#define ID_MODULE_TEMP_8 0xbe8
#define ID_MODULE_TEMP_9 0xbe9
#define ID_MODULE_TEMP_10 0xbea
#define ID_MODULE_TEMP_11 0xbeb
#define ID_MODULE_TEMP_12 0xbec
#define ID_MODULE_TEMP_13 0xbed
#define ID_MODULE_TEMP_14 0xbee
#define ID_MODULE_TEMP_15 0xbef
#define ID_ELCON_CHARGER_STATUS 0x18ff50e5
#define ID_ORION_INFO 0x140006b8
#define ID_ORION_CURRENTS_VOLTS 0x140006f8
#define ID_ORION_ERRORS 0xc000738
#define ID_PRECHARGE_BL_CMD 0x409c57e
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_FAULT_SYNC_DRIVELINE 0x8ca83
#define ID_FAULT_SYNC_DASHBOARD 0x8cb05
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8ca42
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_PRECHARGE 0x14000132
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_HEAT_REQ 3
#define DLC_PACK_CURR 2
#define DLC_BALANCE_REQUEST 2
#define DLC_PRECHARGE_HB 2
#define DLC_ELCON_CHARGER_COMMAND 5
#define DLC_NUM_THEM_BAD 4
#define DLC_PACK_CHARGE_STATUS 7
#define DLC_GYRO_DATA 6
#define DLC_ACCEL_DATA 6
#define DLC_ANGLE_DATA 6
#define DLC_MAX_CELL_TEMP 2
#define DLC_MOD_CELL_TEMP_AVG 8
#define DLC_MOD_CELL_TEMP_MAX 8
#define DLC_MOD_CELL_TEMP_MIN 8
#define DLC_RAW_CELL_TEMP 7
#define DLC_FAULT_SYNC_PRECHARGE 3
#define DLC_DAQ_RESPONSE_PRECHARGE 8
#define DLC_MODULE_TEMP_0 8
#define DLC_MODULE_TEMP_1 8
#define DLC_MODULE_TEMP_2 8
#define DLC_MODULE_TEMP_3 8
#define DLC_MODULE_TEMP_4 8
#define DLC_MODULE_TEMP_5 8
#define DLC_MODULE_TEMP_6 8
#define DLC_MODULE_TEMP_7 8
#define DLC_MODULE_TEMP_8 8
#define DLC_MODULE_TEMP_9 8
#define DLC_MODULE_TEMP_10 8
#define DLC_MODULE_TEMP_11 8
#define DLC_MODULE_TEMP_12 8
#define DLC_MODULE_TEMP_13 8
#define DLC_MODULE_TEMP_14 8
#define DLC_MODULE_TEMP_15 8
#define DLC_ELCON_CHARGER_STATUS 5
#define DLC_ORION_INFO 7
#define DLC_ORION_CURRENTS_VOLTS 4
#define DLC_ORION_ERRORS 4
#define DLC_PRECHARGE_BL_CMD 5
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DRIVELINE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_PRECHARGE 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_HEAT_REQ(queue, toggle_, time_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_HEAT_REQ, .DLC=DLC_HEAT_REQ, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->heat_req.toggle = toggle_;\
        data_a->heat_req.time = time_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_CURR(queue, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_PACK_CURR, .DLC=DLC_PACK_CURR, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_curr.current = current_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BALANCE_REQUEST(queue, voltage_target_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_BALANCE_REQUEST, .DLC=DLC_BALANCE_REQUEST, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->balance_request.voltage_target = voltage_target_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PRECHARGE_HB(queue, IMD_, BMS_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PRECHARGE_HB, .DLC=DLC_PRECHARGE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->precharge_hb.IMD = IMD_;\
        data_a->precharge_hb.BMS = BMS_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_ELCON_CHARGER_COMMAND(queue, voltage_limit_, current_limit_, charge_disable_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ELCON_CHARGER_COMMAND, .DLC=DLC_ELCON_CHARGER_COMMAND, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->elcon_charger_command.voltage_limit = voltage_limit_;\
        data_a->elcon_charger_command.current_limit = current_limit_;\
        data_a->elcon_charger_command.charge_disable = charge_disable_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_NUM_THEM_BAD(queue, module_1_, module_2_, module_3_, module_4_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_NUM_THEM_BAD, .DLC=DLC_NUM_THEM_BAD, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->num_them_bad.module_1 = module_1_;\
        data_a->num_them_bad.module_2 = module_2_;\
        data_a->num_them_bad.module_3 = module_3_;\
        data_a->num_them_bad.module_4 = module_4_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_CHARGE_STATUS(queue, power_, charge_enable_, voltage_, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_CHARGE_STATUS, .DLC=DLC_PACK_CHARGE_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_charge_status.power = power_;\
        data_a->pack_charge_status.charge_enable = charge_enable_;\
        data_a->pack_charge_status.voltage = voltage_;\
        data_a->pack_charge_status.current = current_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_GYRO_DATA(queue, gx_, gy_, gz_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_GYRO_DATA, .DLC=DLC_GYRO_DATA, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->gyro_data.gx = gx_;\
        data_a->gyro_data.gy = gy_;\
        data_a->gyro_data.gz = gz_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_ACCEL_DATA(queue, ax_, ay_, az_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ACCEL_DATA, .DLC=DLC_ACCEL_DATA, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->accel_data.ax = ax_;\
        data_a->accel_data.ay = ay_;\
        data_a->accel_data.az = az_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_ANGLE_DATA(queue, pitch_, roll_, yaw_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ANGLE_DATA, .DLC=DLC_ANGLE_DATA, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->angle_data.pitch = pitch_;\
        data_a->angle_data.roll = roll_;\
        data_a->angle_data.yaw = yaw_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_MAX_CELL_TEMP(queue, max_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAX_CELL_TEMP, .DLC=DLC_MAX_CELL_TEMP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->max_cell_temp.max_temp = max_temp_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_AVG(queue, temp_A_, temp_B_, temp_C_, temp_D_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_AVG, .DLC=DLC_MOD_CELL_TEMP_AVG, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_avg.temp_A = temp_A_;\
        data_a->mod_cell_temp_avg.temp_B = temp_B_;\
        data_a->mod_cell_temp_avg.temp_C = temp_C_;\
        data_a->mod_cell_temp_avg.temp_D = temp_D_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_MAX(queue, temp_A_, temp_B_, temp_C_, temp_D_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_MAX, .DLC=DLC_MOD_CELL_TEMP_MAX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_max.temp_A = temp_A_;\
        data_a->mod_cell_temp_max.temp_B = temp_B_;\
        data_a->mod_cell_temp_max.temp_C = temp_C_;\
        data_a->mod_cell_temp_max.temp_D = temp_D_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_MIN(queue, temp_A_, temp_B_, temp_C_, temp_D_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_MIN, .DLC=DLC_MOD_CELL_TEMP_MIN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_min.temp_A = temp_A_;\
        data_a->mod_cell_temp_min.temp_B = temp_B_;\
        data_a->mod_cell_temp_min.temp_C = temp_C_;\
        data_a->mod_cell_temp_min.temp_D = temp_D_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_RAW_CELL_TEMP(queue, index_, temp_A_, temp_B_, temp_C_, temp_D_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_CELL_TEMP, .DLC=DLC_RAW_CELL_TEMP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_cell_temp.index = index_;\
        data_a->raw_cell_temp.temp_A = temp_A_;\
        data_a->raw_cell_temp.temp_B = temp_B_;\
        data_a->raw_cell_temp.temp_C = temp_C_;\
        data_a->raw_cell_temp.temp_D = temp_D_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_FAULT_SYNC_PRECHARGE(queue, idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_PRECHARGE, .DLC=DLC_FAULT_SYNC_PRECHARGE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_precharge.idx = idx_;\
        data_a->fault_sync_precharge.latched = latched_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_PRECHARGE(queue, daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_PRECHARGE, .DLC=DLC_DAQ_RESPONSE_PRECHARGE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_PRECHARGE.daq_response = daq_response_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_ELCON_CHARGER_STATUS 2000
#define UP_ORION_INFO 32
#define UP_ORION_CURRENTS_VOLTS 32
#define UP_ORION_ERRORS 1000
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t toggle: 1;
        uint64_t time: 16;
    } heat_req;
    struct {
        uint64_t current: 16;
    } pack_curr;
    struct {
        uint64_t voltage_target: 16;
    } balance_request;
    struct {
        uint64_t IMD: 8;
        uint64_t BMS: 8;
    } precharge_hb;
    struct {
        uint64_t voltage_limit: 16;
        uint64_t current_limit: 16;
        uint64_t charge_disable: 1;
    } elcon_charger_command;
    struct {
        uint64_t module_1: 8;
        uint64_t module_2: 8;
        uint64_t module_3: 8;
        uint64_t module_4: 8;
    } num_them_bad;
    struct {
        uint64_t power: 16;
        uint64_t charge_enable: 1;
        uint64_t voltage: 16;
        uint64_t current: 16;
    } pack_charge_status;
    struct {
        uint64_t gx: 16;
        uint64_t gy: 16;
        uint64_t gz: 16;
    } gyro_data;
    struct {
        uint64_t ax: 16;
        uint64_t ay: 16;
        uint64_t az: 16;
    } accel_data;
    struct {
        uint64_t pitch: 16;
        uint64_t roll: 16;
        uint64_t yaw: 16;
    } angle_data;
    struct {
        uint64_t max_temp: 16;
    } max_cell_temp;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
        uint64_t temp_D: 16;
    } mod_cell_temp_avg;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
        uint64_t temp_D: 16;
    } mod_cell_temp_max;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
        uint64_t temp_D: 16;
    } mod_cell_temp_min;
    struct {
        uint64_t index: 8;
        uint64_t temp_A: 12;
        uint64_t temp_B: 12;
        uint64_t temp_C: 12;
        uint64_t temp_D: 12;
    } raw_cell_temp;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_precharge;
    struct {
        uint64_t daq_response: 64;
    } daq_response_PRECHARGE;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_0;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_1;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_2;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_3;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_4;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_5;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_6;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_7;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_8;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_9;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_10;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_11;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_12;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_13;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_14;
    struct {
        uint64_t mod_temp_0: 16;
        uint64_t mod_temp_1: 16;
        uint64_t mod_temp_2: 16;
        uint64_t mod_temp_3: 16;
    } module_temp_15;
    struct {
        uint64_t charge_voltage: 16;
        uint64_t charge_current: 16;
        uint64_t hw_fail: 1;
        uint64_t temp_fail: 1;
        uint64_t input_v_fail: 1;
        uint64_t startup_fail: 1;
        uint64_t communication_fail: 1;
    } elcon_charger_status;
    struct {
        uint64_t discharge_enable: 1;
        uint64_t charge_enable: 1;
        uint64_t charger_safety: 1;
        uint64_t dtc_status: 1;
        uint64_t multi_input: 1;
        uint64_t always_on: 1;
        uint64_t is_ready: 1;
        uint64_t is_charging: 1;
        uint64_t multi_input_2: 1;
        uint64_t multi_input_3: 1;
        uint64_t reserved: 1;
        uint64_t multi_output_2: 1;
        uint64_t multi_output_3: 1;
        uint64_t multi_output_4: 1;
        uint64_t multi_enable: 1;
        uint64_t multi_output_1: 1;
        uint64_t pack_dcl: 16;
        uint64_t pack_ccl: 16;
        uint64_t pack_soc: 8;
    } orion_info;
    struct {
        uint64_t pack_current: 16;
        uint64_t pack_voltage: 16;
    } orion_currents_volts;
    struct {
        uint64_t discharge_limit_enforce: 1;
        uint64_t charger_safety_relay: 1;
        uint64_t internal_hardware: 1;
        uint64_t heatsink_thermistor: 1;
        uint64_t software: 1;
        uint64_t max_cellv_high: 1;
        uint64_t min_cellv_low: 1;
        uint64_t pack_overheat: 1;
        uint64_t reserved0: 1;
        uint64_t reserved1: 1;
        uint64_t reserved2: 1;
        uint64_t reserved3: 1;
        uint64_t reserved4: 1;
        uint64_t reserved5: 1;
        uint64_t reserved6: 1;
        uint64_t reserved7: 1;
        uint64_t internal_comms: 1;
        uint64_t cell_balancing_foff: 1;
        uint64_t weak_cell: 1;
        uint64_t low_cellv: 1;
        uint64_t open_wire: 1;
        uint64_t current_sensor: 1;
        uint64_t max_cellv_o5v: 1;
        uint64_t cell_asic: 1;
        uint64_t weak_pack: 1;
        uint64_t fan_monitor: 1;
        uint64_t thermistor: 1;
        uint64_t external_comms: 1;
        uint64_t redundant_psu: 1;
        uint64_t hv_isolation: 1;
        uint64_t input_psu: 1;
        uint64_t charge_limit_enforce: 1;
    } orion_errors;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } precharge_bl_cmd;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
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
    } daq_command_PRECHARGE;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_0;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_1;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_2;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_3;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_4;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_5;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_6;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_7;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_8;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_9;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_10;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_11;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_12;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_13;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_14;
    struct {
        uint16_t mod_temp_0;
        uint16_t mod_temp_1;
        uint16_t mod_temp_2;
        uint16_t mod_temp_3;
    } module_temp_15;
    struct {
        uint16_t charge_voltage;
        uint16_t charge_current;
        uint8_t hw_fail;
        uint8_t temp_fail;
        uint8_t input_v_fail;
        uint8_t startup_fail;
        uint8_t communication_fail;
        uint8_t stale;
        uint32_t last_rx;
    } elcon_charger_status;
    struct {
        uint8_t discharge_enable;
        uint8_t charge_enable;
        uint8_t charger_safety;
        uint8_t dtc_status;
        uint8_t multi_input;
        uint8_t always_on;
        uint8_t is_ready;
        uint8_t is_charging;
        uint8_t multi_input_2;
        uint8_t multi_input_3;
        uint8_t reserved;
        uint8_t multi_output_2;
        uint8_t multi_output_3;
        uint8_t multi_output_4;
        uint8_t multi_enable;
        uint8_t multi_output_1;
        uint16_t pack_dcl;
        uint16_t pack_ccl;
        uint8_t pack_soc;
        uint8_t stale;
        uint32_t last_rx;
    } orion_info;
    struct {
        int16_t pack_current;
        uint16_t pack_voltage;
        uint8_t stale;
        uint32_t last_rx;
    } orion_currents_volts;
    struct {
        uint8_t discharge_limit_enforce;
        uint8_t charger_safety_relay;
        uint8_t internal_hardware;
        uint8_t heatsink_thermistor;
        uint8_t software;
        uint8_t max_cellv_high;
        uint8_t min_cellv_low;
        uint8_t pack_overheat;
        uint8_t reserved0;
        uint8_t reserved1;
        uint8_t reserved2;
        uint8_t reserved3;
        uint8_t reserved4;
        uint8_t reserved5;
        uint8_t reserved6;
        uint8_t reserved7;
        uint8_t internal_comms;
        uint8_t cell_balancing_foff;
        uint8_t weak_cell;
        uint8_t low_cellv;
        uint8_t open_wire;
        uint8_t current_sensor;
        uint8_t max_cellv_o5v;
        uint8_t cell_asic;
        uint8_t weak_pack;
        uint8_t fan_monitor;
        uint8_t thermistor;
        uint8_t external_comms;
        uint8_t redundant_psu;
        uint8_t hv_isolation;
        uint8_t input_psu;
        uint8_t charge_limit_enforce;
        uint8_t stale;
        uint32_t last_rx;
    } orion_errors;
    struct {
        uint8_t cmd;
        uint32_t data;
    } precharge_bl_cmd;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_main_module;
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
    } daq_command_PRECHARGE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_PRECHARGE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void precharge_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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

extern volatile uint32_t last_can_rx_time_ms;

#endif