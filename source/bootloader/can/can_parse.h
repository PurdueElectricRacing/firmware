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
#define NODE_NAME "bootloader"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_MAIN_MODULE_BL_RESP 0x404e23c
#define ID_DASHBOARD_BL_RESP 0x404e27c
#define ID_TORQUEVECTOR_BL_RESP 0x404e2bc
#define ID_DRIVELINE_FRONT_BL_RESP 0x404e2fc
#define ID_DRIVELINE_REAR_BL_RESP 0x404e33c
#define ID_PRECHARGE_BL_RESP 0x404e37c
#define ID_BMS_A_BL_RESP 0x404e3bc
#define ID_BMS_B_BL_RESP 0x404e3fc
#define ID_BMS_C_BL_RESP 0x404e43c
#define ID_BMS_D_BL_RESP 0x404e47c
#define ID_BMS_E_BL_RESP 0x404e4bc
#define ID_BMS_F_BL_RESP 0x404e4fc
#define ID_BMS_G_BL_RESP 0x404e53c
#define ID_BMS_H_BL_RESP 0x404e57c
#define ID_BMS_I_BL_RESP 0x404e5bc
#define ID_BMS_J_BL_RESP 0x404e5fc
#define ID_L4_TESTING_BL_RESP 0x404e63c
#define ID_BITSTREAM_DATA 0x400193e
#define ID_MAIN_MODULE_BL_CMD 0x409c43e
#define ID_DASHBOARD_BL_CMD 0x409c47e
#define ID_TORQUEVECTOR_BL_CMD 0x409c4be
#define ID_DRIVELINE_FRONT_BL_CMD 0x409c4fe
#define ID_DRIVELINE_REAR_BL_CMD 0x409c53e
#define ID_PRECHARGE_BL_CMD 0x409c57e
#define ID_BMS_A_BL_CMD 0x409c5be
#define ID_BMS_B_BL_CMD 0x409c5fe
#define ID_BMS_C_BL_CMD 0x409c63e
#define ID_BMS_D_BL_CMD 0x409c67e
#define ID_BMS_E_BL_CMD 0x409c6be
#define ID_BMS_F_BL_CMD 0x409c6fe
#define ID_BMS_G_BL_CMD 0x409c73e
#define ID_BMS_H_BL_CMD 0x409c77e
#define ID_BMS_I_BL_CMD 0x409c7be
#define ID_BMS_J_BL_CMD 0x409c7fe
#define ID_L4_TESTING_BL_CMD 0x409c83e
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAIN_MODULE_BL_RESP 5
#define DLC_DASHBOARD_BL_RESP 5
#define DLC_TORQUEVECTOR_BL_RESP 5
#define DLC_DRIVELINE_FRONT_BL_RESP 5
#define DLC_DRIVELINE_REAR_BL_RESP 5
#define DLC_PRECHARGE_BL_RESP 5
#define DLC_BMS_A_BL_RESP 5
#define DLC_BMS_B_BL_RESP 5
#define DLC_BMS_C_BL_RESP 5
#define DLC_BMS_D_BL_RESP 5
#define DLC_BMS_E_BL_RESP 5
#define DLC_BMS_F_BL_RESP 5
#define DLC_BMS_G_BL_RESP 5
#define DLC_BMS_H_BL_RESP 5
#define DLC_BMS_I_BL_RESP 5
#define DLC_BMS_J_BL_RESP 5
#define DLC_L4_TESTING_BL_RESP 5
#define DLC_BITSTREAM_DATA 8
#define DLC_MAIN_MODULE_BL_CMD 5
#define DLC_DASHBOARD_BL_CMD 5
#define DLC_TORQUEVECTOR_BL_CMD 5
#define DLC_DRIVELINE_FRONT_BL_CMD 5
#define DLC_DRIVELINE_REAR_BL_CMD 5
#define DLC_PRECHARGE_BL_CMD 5
#define DLC_BMS_A_BL_CMD 5
#define DLC_BMS_B_BL_CMD 5
#define DLC_BMS_C_BL_CMD 5
#define DLC_BMS_D_BL_CMD 5
#define DLC_BMS_E_BL_CMD 5
#define DLC_BMS_F_BL_CMD 5
#define DLC_BMS_G_BL_CMD 5
#define DLC_BMS_H_BL_CMD 5
#define DLC_BMS_I_BL_CMD 5
#define DLC_BMS_J_BL_CMD 5
#define DLC_L4_TESTING_BL_CMD 5
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAIN_MODULE_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_MODULE_BL_RESP, .DLC=DLC_MAIN_MODULE_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_module_bl_resp.cmd = cmd_;\
        data_a->main_module_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DASHBOARD_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DASHBOARD_BL_RESP, .DLC=DLC_DASHBOARD_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->dashboard_bl_resp.cmd = cmd_;\
        data_a->dashboard_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TORQUEVECTOR_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TORQUEVECTOR_BL_RESP, .DLC=DLC_TORQUEVECTOR_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->torquevector_bl_resp.cmd = cmd_;\
        data_a->torquevector_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DRIVELINE_FRONT_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DRIVELINE_FRONT_BL_RESP, .DLC=DLC_DRIVELINE_FRONT_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->driveline_front_bl_resp.cmd = cmd_;\
        data_a->driveline_front_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_DRIVELINE_REAR_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DRIVELINE_REAR_BL_RESP, .DLC=DLC_DRIVELINE_REAR_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->driveline_rear_bl_resp.cmd = cmd_;\
        data_a->driveline_rear_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PRECHARGE_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PRECHARGE_BL_RESP, .DLC=DLC_PRECHARGE_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->precharge_bl_resp.cmd = cmd_;\
        data_a->precharge_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_A_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_A_BL_RESP, .DLC=DLC_BMS_A_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_a_bl_resp.cmd = cmd_;\
        data_a->bms_a_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_B_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_B_BL_RESP, .DLC=DLC_BMS_B_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_b_bl_resp.cmd = cmd_;\
        data_a->bms_b_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_C_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_C_BL_RESP, .DLC=DLC_BMS_C_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_c_bl_resp.cmd = cmd_;\
        data_a->bms_c_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_D_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_D_BL_RESP, .DLC=DLC_BMS_D_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_d_bl_resp.cmd = cmd_;\
        data_a->bms_d_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_E_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_E_BL_RESP, .DLC=DLC_BMS_E_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_e_bl_resp.cmd = cmd_;\
        data_a->bms_e_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_F_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_F_BL_RESP, .DLC=DLC_BMS_F_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_f_bl_resp.cmd = cmd_;\
        data_a->bms_f_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_G_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_G_BL_RESP, .DLC=DLC_BMS_G_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_g_bl_resp.cmd = cmd_;\
        data_a->bms_g_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_H_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_H_BL_RESP, .DLC=DLC_BMS_H_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_h_bl_resp.cmd = cmd_;\
        data_a->bms_h_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_I_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_I_BL_RESP, .DLC=DLC_BMS_I_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_i_bl_resp.cmd = cmd_;\
        data_a->bms_i_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_BMS_J_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_BMS_J_BL_RESP, .DLC=DLC_BMS_J_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->bms_j_bl_resp.cmd = cmd_;\
        data_a->bms_j_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_L4_TESTING_BL_RESP(queue, cmd_, data_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_L4_TESTING_BL_RESP, .DLC=DLC_L4_TESTING_BL_RESP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->l4_testing_bl_resp.cmd = cmd_;\
        data_a->l4_testing_bl_resp.data = data_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
/* END AUTO CAN ENUMERATIONS */

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { 
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } main_module_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } dashboard_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } torquevector_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } driveline_front_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } driveline_rear_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } precharge_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_a_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_b_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_c_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_d_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_e_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_f_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_g_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_h_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_i_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_j_bl_resp;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } l4_testing_bl_resp;
    struct {
        uint64_t d0: 8;
        uint64_t d1: 8;
        uint64_t d2: 8;
        uint64_t d3: 8;
        uint64_t d4: 8;
        uint64_t d5: 8;
        uint64_t d6: 8;
        uint64_t d7: 8;
    } bitstream_data;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } main_module_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } dashboard_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } torquevector_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } driveline_front_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } driveline_rear_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } precharge_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_a_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_b_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_c_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_d_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_e_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_f_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_g_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_h_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_i_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } bms_j_bl_cmd;
    struct {
        uint64_t cmd: 8;
        uint64_t data: 32;
    } l4_testing_bl_cmd;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t d0;
        uint8_t d1;
        uint8_t d2;
        uint8_t d3;
        uint8_t d4;
        uint8_t d5;
        uint8_t d6;
        uint8_t d7;
    } bitstream_data;
    struct {
        uint8_t cmd;
        uint32_t data;
    } main_module_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } dashboard_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } torquevector_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } driveline_front_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } driveline_rear_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } precharge_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_a_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_b_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_c_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_d_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_e_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_f_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_g_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_h_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_i_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } bms_j_bl_cmd;
    struct {
        uint8_t cmd;
        uint32_t data;
    } l4_testing_bl_cmd;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void bitstream_data_CALLBACK(CanParsedData_t* msg_data_a);
extern void main_module_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void dashboard_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void torquevector_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void driveline_front_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void driveline_rear_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void precharge_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_a_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_b_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_c_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_d_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_e_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_f_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_g_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_h_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_i_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void bms_j_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
extern void l4_testing_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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