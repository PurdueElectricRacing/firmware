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
#define ID_BATTERY_INFO 0x8008004
#define ID_CELL_INFO 0x8008044
#define ID_ELCON_CHARGER_COMMAND 0x1806e5f4
#define ID_PACK_CHARGE_STATUS 0x8008084
#define ID_GYRO_DATA 0x4008004
#define ID_ACCEL_DATA 0x4008044
#define ID_DAQ_RESPONSE_PRECHARGE 0x17ffffc4
#define ID_SOC_CELLS_1 0x8007d6b
#define ID_VOLTS_CELLS_1 0x4007dab
#define ID_PACK_INFO_1 0x8007deb
#define ID_TEMPS_CELLS_1 0x4007e2b
#define ID_CELL_INFO_1 0x8007e6b
#define ID_POWER_LIM_1 0x4007eab
#define ID_SOC_CELLS_2 0x8007eeb
#define ID_VOLTS_CELLS_2 0x4007f2b
#define ID_PACK_INFO_2 0x8007f6b
#define ID_TEMPS_CELLS_2 0x4007fab
#define ID_CELL_INFO_2 0x8007feb
#define ID_POWER_LIM_2 0x400802b
#define ID_SOC_CELLS_3 0x800806b
#define ID_VOLTS_CELLS_3 0x40080ab
#define ID_PACK_INFO_3 0x80080eb
#define ID_TEMPS_CELLS_3 0x400812b
#define ID_CELL_INFO_3 0x800816b
#define ID_POWER_LIM_3 0x40081ab
#define ID_SOC_CELLS_4 0x80081eb
#define ID_VOLTS_CELLS_4 0x400822b
#define ID_PACK_INFO_4 0x800826b
#define ID_TEMPS_CELLS_4 0x40082ab
#define ID_CELL_INFO_4 0x80082eb
#define ID_POWER_LIM_4 0x400832b
#define ID_SOC_CELLS_5 0x800836b
#define ID_VOLTS_CELLS_5 0x40083ab
#define ID_PACK_INFO_5 0x80083eb
#define ID_TEMPS_CELLS_5 0x400842b
#define ID_CELL_INFO_5 0x800846b
#define ID_POWER_LIM_5 0x40084ab
#define ID_SOC_CELLS_6 0x80084eb
#define ID_VOLTS_CELLS_6 0x400852b
#define ID_PACK_INFO_6 0x800856b
#define ID_TEMPS_CELLS_6 0x40085ab
#define ID_CELL_INFO_6 0x80085eb
#define ID_POWER_LIM_6 0x400862b
#define ID_SOC_CELLS_7 0x800866b
#define ID_VOLTS_CELLS_7 0x40086ab
#define ID_PACK_INFO_7 0x80086eb
#define ID_TEMPS_CELLS_7 0x400872b
#define ID_CELL_INFO_7 0x800876b
#define ID_POWER_LIM_7 0x40087ab
#define ID_SOC_CELLS_8 0x80087eb
#define ID_VOLTS_CELLS_8 0x400882b
#define ID_PACK_INFO_8 0x800886b
#define ID_TEMPS_CELLS_8 0x40088ab
#define ID_CELL_INFO_8 0x80088eb
#define ID_POWER_LIM_8 0x400892b
#define ID_ELCON_CHARGER_STATUS 0x18ff50e5
#define ID_DAQ_COMMAND_PRECHARGE 0x14000132
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_HEAT_REQ 3
#define DLC_PACK_CURR 2
#define DLC_BALANCE_REQUEST 2
#define DLC_BATTERY_INFO 8
#define DLC_CELL_INFO 7
#define DLC_ELCON_CHARGER_COMMAND 5
#define DLC_PACK_CHARGE_STATUS 3
#define DLC_GYRO_DATA 6
#define DLC_ACCEL_DATA 6
#define DLC_DAQ_RESPONSE_PRECHARGE 8
#define DLC_SOC_CELLS_1 7
#define DLC_VOLTS_CELLS_1 7
#define DLC_PACK_INFO_1 6
#define DLC_TEMPS_CELLS_1 7
#define DLC_CELL_INFO_1 6
#define DLC_POWER_LIM_1 4
#define DLC_SOC_CELLS_2 7
#define DLC_VOLTS_CELLS_2 7
#define DLC_PACK_INFO_2 6
#define DLC_TEMPS_CELLS_2 7
#define DLC_CELL_INFO_2 6
#define DLC_POWER_LIM_2 4
#define DLC_SOC_CELLS_3 7
#define DLC_VOLTS_CELLS_3 7
#define DLC_PACK_INFO_3 6
#define DLC_TEMPS_CELLS_3 7
#define DLC_CELL_INFO_3 6
#define DLC_POWER_LIM_3 4
#define DLC_SOC_CELLS_4 7
#define DLC_VOLTS_CELLS_4 7
#define DLC_PACK_INFO_4 6
#define DLC_TEMPS_CELLS_4 7
#define DLC_CELL_INFO_4 6
#define DLC_POWER_LIM_4 4
#define DLC_SOC_CELLS_5 7
#define DLC_VOLTS_CELLS_5 7
#define DLC_PACK_INFO_5 6
#define DLC_TEMPS_CELLS_5 7
#define DLC_CELL_INFO_5 6
#define DLC_POWER_LIM_5 4
#define DLC_SOC_CELLS_6 7
#define DLC_VOLTS_CELLS_6 7
#define DLC_PACK_INFO_6 6
#define DLC_TEMPS_CELLS_6 7
#define DLC_CELL_INFO_6 6
#define DLC_POWER_LIM_6 4
#define DLC_SOC_CELLS_7 7
#define DLC_VOLTS_CELLS_7 7
#define DLC_PACK_INFO_7 6
#define DLC_TEMPS_CELLS_7 7
#define DLC_CELL_INFO_7 6
#define DLC_POWER_LIM_7 4
#define DLC_SOC_CELLS_8 7
#define DLC_VOLTS_CELLS_8 7
#define DLC_PACK_INFO_8 6
#define DLC_TEMPS_CELLS_8 7
#define DLC_CELL_INFO_8 6
#define DLC_POWER_LIM_8 4
#define DLC_ELCON_CHARGER_STATUS 5
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
#define SEND_BATTERY_INFO(queue, voltage_, delta_, lowest_, error_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BATTERY_INFO, .DLC=DLC_BATTERY_INFO, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->battery_info.voltage = voltage_;\
        data_a->battery_info.delta = delta_;\
        data_a->battery_info.lowest = lowest_;\
        data_a->battery_info.error = error_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO, .DLC=DLC_CELL_INFO, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info.idx = idx_;\
        data_a->cell_info.v1 = v1_;\
        data_a->cell_info.v2 = v2_;\
        data_a->cell_info.v3 = v3_;\
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
#define SEND_PACK_CHARGE_STATUS(queue, power_, charge_enable_, balance_enable_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_CHARGE_STATUS, .DLC=DLC_PACK_CHARGE_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_charge_status.power = power_;\
        data_a->pack_charge_status.charge_enable = charge_enable_;\
        data_a->pack_charge_status.balance_enable = balance_enable_;\
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
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
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
        uint64_t voltage: 16;
        uint64_t delta: 16;
        uint64_t lowest: 16;
        uint64_t error: 16;
    } battery_info;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } cell_info;
    struct {
        uint64_t voltage_limit: 16;
        uint64_t current_limit: 16;
        uint64_t charge_disable: 1;
    } elcon_charger_command;
    struct {
        uint64_t power: 16;
        uint64_t charge_enable: 1;
        uint64_t balance_enable: 1;
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
        uint64_t daq_response: 64;
    } daq_response_PRECHARGE;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_1;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_1;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_1;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_1;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_1;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_1;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_2;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_2;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_2;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_2;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_2;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_2;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_3;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_3;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_3;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_3;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_3;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_3;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_4;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_4;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_4;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_4;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_4;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_4;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_5;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_5;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_5;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_5;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_5;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_5;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_6;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_6;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_6;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_6;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_6;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_6;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_7;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_7;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_7;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_7;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_7;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_7;
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells_8;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells_8;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info_8;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells_8;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info_8;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim_8;
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
        uint64_t daq_command: 64;
    } daq_command_PRECHARGE;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_1;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_1;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_1;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_1;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_1;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_1;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_2;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_2;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_2;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_2;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_2;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_2;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_3;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_3;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_3;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_3;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_3;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_3;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_4;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_4;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_4;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_4;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_4;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_4;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_5;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_5;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_5;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_5;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_5;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_5;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_6;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_6;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_6;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_6;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_6;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_6;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_7;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_7;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_7;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_7;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_7;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_7;
    struct {
        uint8_t idx;
        uint16_t soc1;
        uint16_t soc2;
        uint16_t soc3;
    } soc_cells_8;
    struct {
        uint8_t idx;
        uint16_t v1;
        uint16_t v2;
        uint16_t v3;
    } volts_cells_8;
    struct {
        uint16_t volts;
        uint16_t error;
        uint16_t bal_flags;
    } pack_info_8;
    struct {
        uint8_t idx;
        uint16_t t1;
        uint16_t t2;
        uint16_t t3;
    } temps_cells_8;
    struct {
        uint16_t delta;
        uint16_t ov;
        uint16_t uv;
    } cell_info_8;
    struct {
        uint16_t disch_lim;
        uint16_t chg_lim;
    } power_lim_8;
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
        uint64_t daq_command;
    } daq_command_PRECHARGE;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_PRECHARGE_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void volts_cells_1_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_2_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_3_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_4_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_5_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_6_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_7_CALLBACK(CanParsedData_t* msg_data_a);
extern void volts_cells_8_CALLBACK(CanParsedData_t* msg_data_a);
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