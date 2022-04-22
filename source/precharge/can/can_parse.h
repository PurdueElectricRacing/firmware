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
#define ID_TEST_PRECHARGE_MSG 0x8008004
#define ID_HEAT_REQ 0x8007d2a
#define ID_PACK_CURR 0x4007d6a
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
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_TEST_PRECHARGE_MSG 1
#define DLC_HEAT_REQ 3
#define DLC_PACK_CURR 2
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
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_TEST_PRECHARGE_MSG(queue, test_precharge_sig_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEST_PRECHARGE_MSG, .DLC=DLC_TEST_PRECHARGE_MSG, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->test_precharge_msg.test_precharge_sig = test_precharge_sig_;\
        qSendToBack(&queue, &msg);\
    } while(0)
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
typedef union { __attribute__((packed))
    struct {
        uint64_t test_precharge_sig: 8;
    } test_precharge_msg;
    struct {
        uint64_t toggle: 1;
        uint64_t time: 16;
    } heat_req;
    struct {
        uint64_t current: 16;
    } pack_curr;
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
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
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