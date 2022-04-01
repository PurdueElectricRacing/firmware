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
#include "common/phal_L4/can/can.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "BMS1"

#define RX_UPDATE_PERIOD 15 // ms

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_SOC_CELLS 0x8007d6b
#define ID_VOLTS_CELLS 0x4007dab
#define ID_PACK_INFO 0x8007deb
#define ID_TEMPS_CELLS 0x4007e2b
#define ID_CELL_INFO 0x8007e6b
#define ID_POWER_LIM 0x4007eab
#define ID_HEAT_REQ 0x8007d2a
#define ID_PACK_CURR 0x4007d6a
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_SOC_CELLS 7
#define DLC_VOLTS_CELLS 7
#define DLC_PACK_INFO 6
#define DLC_TEMPS_CELLS 7
#define DLC_CELL_INFO 6
#define DLC_POWER_LIM 4
#define DLC_HEAT_REQ 3
#define DLC_PACK_CURR 2
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_SOC_CELLS(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS, .DLC=DLC_SOC_CELLS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells.idx = idx_;\
        data_a->soc_cells.soc1 = soc1_;\
        data_a->soc_cells.soc2 = soc2_;\
        data_a->soc_cells.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS, .DLC=DLC_VOLTS_CELLS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells.idx = idx_;\
        data_a->volts_cells.v1 = v1_;\
        data_a->volts_cells.v2 = v2_;\
        data_a->volts_cells.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO, .DLC=DLC_PACK_INFO, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info.volts = volts_;\
        data_a->pack_info.error = error_;\
        data_a->pack_info.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS, .DLC=DLC_TEMPS_CELLS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells.idx = idx_;\
        data_a->temps_cells.t1 = t1_;\
        data_a->temps_cells.t2 = t2_;\
        data_a->temps_cells.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO, .DLC=DLC_CELL_INFO, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info.delta = delta_;\
        data_a->cell_info.ov = ov_;\
        data_a->cell_info.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM, .DLC=DLC_POWER_LIM, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim.disch_lim = disch_lim_;\
        data_a->power_lim.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 3 / 2 // 3 / 2 would be 150% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) * RX_UPDATE_PERIOD > period * STALE_THRESH) stale = 1

// Message Raw Structures
/* BEGIN AUTO MESSAGE STRUCTURE */
typedef union { __attribute__((packed))
    struct {
        uint64_t idx: 8;
        uint64_t soc1: 16;
        uint64_t soc2: 16;
        uint64_t soc3: 16;
    } soc_cells;
    struct {
        uint64_t idx: 8;
        uint64_t v1: 16;
        uint64_t v2: 16;
        uint64_t v3: 16;
    } volts_cells;
    struct {
        uint64_t volts: 16;
        uint64_t error: 16;
        uint64_t bal_flags: 16;
    } pack_info;
    struct {
        uint64_t idx: 8;
        uint64_t t1: 16;
        uint64_t t2: 16;
        uint64_t t3: 16;
    } temps_cells;
    struct {
        uint64_t delta: 16;
        uint64_t ov: 16;
        uint64_t uv: 16;
    } cell_info;
    struct {
        uint64_t disch_lim: 16;
        uint64_t chg_lim: 16;
    } power_lim;
    struct {
        uint64_t toggle: 1;
        uint64_t time: 16;
    } heat_req;
    struct {
        uint64_t current: 16;
    } pack_curr;
    uint8_t raw_data[8];
} CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        uint8_t toggle;
        uint16_t time;
    } heat_req;
    struct {
        int16_t current;
    } pack_curr;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
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