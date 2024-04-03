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
#define NODE_NAME "BMS"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
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
#define ID_HEAT_REQ 0x8007d2a
#define ID_PACK_CURR 0x4007d6a
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
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
#define DLC_HEAT_REQ 3
#define DLC_PACK_CURR 2
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_SOC_CELLS_1(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_1, .DLC=DLC_SOC_CELLS_1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_1.idx = idx_;\
        data_a->soc_cells_1.soc1 = soc1_;\
        data_a->soc_cells_1.soc2 = soc2_;\
        data_a->soc_cells_1.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_1(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_1, .DLC=DLC_VOLTS_CELLS_1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_1.idx = idx_;\
        data_a->volts_cells_1.v1 = v1_;\
        data_a->volts_cells_1.v2 = v2_;\
        data_a->volts_cells_1.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_1(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_1, .DLC=DLC_PACK_INFO_1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_1.volts = volts_;\
        data_a->pack_info_1.error = error_;\
        data_a->pack_info_1.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_1(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_1, .DLC=DLC_TEMPS_CELLS_1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_1.idx = idx_;\
        data_a->temps_cells_1.t1 = t1_;\
        data_a->temps_cells_1.t2 = t2_;\
        data_a->temps_cells_1.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_1(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_1, .DLC=DLC_CELL_INFO_1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_1.delta = delta_;\
        data_a->cell_info_1.ov = ov_;\
        data_a->cell_info_1.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_1(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_1, .DLC=DLC_POWER_LIM_1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_1.disch_lim = disch_lim_;\
        data_a->power_lim_1.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_2(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_2, .DLC=DLC_SOC_CELLS_2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_2.idx = idx_;\
        data_a->soc_cells_2.soc1 = soc1_;\
        data_a->soc_cells_2.soc2 = soc2_;\
        data_a->soc_cells_2.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_2(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_2, .DLC=DLC_VOLTS_CELLS_2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_2.idx = idx_;\
        data_a->volts_cells_2.v1 = v1_;\
        data_a->volts_cells_2.v2 = v2_;\
        data_a->volts_cells_2.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_2(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_2, .DLC=DLC_PACK_INFO_2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_2.volts = volts_;\
        data_a->pack_info_2.error = error_;\
        data_a->pack_info_2.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_2(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_2, .DLC=DLC_TEMPS_CELLS_2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_2.idx = idx_;\
        data_a->temps_cells_2.t1 = t1_;\
        data_a->temps_cells_2.t2 = t2_;\
        data_a->temps_cells_2.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_2(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_2, .DLC=DLC_CELL_INFO_2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_2.delta = delta_;\
        data_a->cell_info_2.ov = ov_;\
        data_a->cell_info_2.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_2(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_2, .DLC=DLC_POWER_LIM_2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_2.disch_lim = disch_lim_;\
        data_a->power_lim_2.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_3(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_3, .DLC=DLC_SOC_CELLS_3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_3.idx = idx_;\
        data_a->soc_cells_3.soc1 = soc1_;\
        data_a->soc_cells_3.soc2 = soc2_;\
        data_a->soc_cells_3.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_3(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_3, .DLC=DLC_VOLTS_CELLS_3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_3.idx = idx_;\
        data_a->volts_cells_3.v1 = v1_;\
        data_a->volts_cells_3.v2 = v2_;\
        data_a->volts_cells_3.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_3(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_3, .DLC=DLC_PACK_INFO_3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_3.volts = volts_;\
        data_a->pack_info_3.error = error_;\
        data_a->pack_info_3.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_3(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_3, .DLC=DLC_TEMPS_CELLS_3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_3.idx = idx_;\
        data_a->temps_cells_3.t1 = t1_;\
        data_a->temps_cells_3.t2 = t2_;\
        data_a->temps_cells_3.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_3(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_3, .DLC=DLC_CELL_INFO_3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_3.delta = delta_;\
        data_a->cell_info_3.ov = ov_;\
        data_a->cell_info_3.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_3(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_3, .DLC=DLC_POWER_LIM_3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_3.disch_lim = disch_lim_;\
        data_a->power_lim_3.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_4(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_4, .DLC=DLC_SOC_CELLS_4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_4.idx = idx_;\
        data_a->soc_cells_4.soc1 = soc1_;\
        data_a->soc_cells_4.soc2 = soc2_;\
        data_a->soc_cells_4.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_4(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_4, .DLC=DLC_VOLTS_CELLS_4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_4.idx = idx_;\
        data_a->volts_cells_4.v1 = v1_;\
        data_a->volts_cells_4.v2 = v2_;\
        data_a->volts_cells_4.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_4(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_4, .DLC=DLC_PACK_INFO_4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_4.volts = volts_;\
        data_a->pack_info_4.error = error_;\
        data_a->pack_info_4.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_4(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_4, .DLC=DLC_TEMPS_CELLS_4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_4.idx = idx_;\
        data_a->temps_cells_4.t1 = t1_;\
        data_a->temps_cells_4.t2 = t2_;\
        data_a->temps_cells_4.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_4(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_4, .DLC=DLC_CELL_INFO_4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_4.delta = delta_;\
        data_a->cell_info_4.ov = ov_;\
        data_a->cell_info_4.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_4(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_4, .DLC=DLC_POWER_LIM_4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_4.disch_lim = disch_lim_;\
        data_a->power_lim_4.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_5(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_5, .DLC=DLC_SOC_CELLS_5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_5.idx = idx_;\
        data_a->soc_cells_5.soc1 = soc1_;\
        data_a->soc_cells_5.soc2 = soc2_;\
        data_a->soc_cells_5.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_5(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_5, .DLC=DLC_VOLTS_CELLS_5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_5.idx = idx_;\
        data_a->volts_cells_5.v1 = v1_;\
        data_a->volts_cells_5.v2 = v2_;\
        data_a->volts_cells_5.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_5(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_5, .DLC=DLC_PACK_INFO_5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_5.volts = volts_;\
        data_a->pack_info_5.error = error_;\
        data_a->pack_info_5.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_5(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_5, .DLC=DLC_TEMPS_CELLS_5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_5.idx = idx_;\
        data_a->temps_cells_5.t1 = t1_;\
        data_a->temps_cells_5.t2 = t2_;\
        data_a->temps_cells_5.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_5(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_5, .DLC=DLC_CELL_INFO_5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_5.delta = delta_;\
        data_a->cell_info_5.ov = ov_;\
        data_a->cell_info_5.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_5(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_5, .DLC=DLC_POWER_LIM_5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_5.disch_lim = disch_lim_;\
        data_a->power_lim_5.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_6(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_6, .DLC=DLC_SOC_CELLS_6, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_6.idx = idx_;\
        data_a->soc_cells_6.soc1 = soc1_;\
        data_a->soc_cells_6.soc2 = soc2_;\
        data_a->soc_cells_6.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_6(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_6, .DLC=DLC_VOLTS_CELLS_6, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_6.idx = idx_;\
        data_a->volts_cells_6.v1 = v1_;\
        data_a->volts_cells_6.v2 = v2_;\
        data_a->volts_cells_6.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_6(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_6, .DLC=DLC_PACK_INFO_6, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_6.volts = volts_;\
        data_a->pack_info_6.error = error_;\
        data_a->pack_info_6.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_6(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_6, .DLC=DLC_TEMPS_CELLS_6, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_6.idx = idx_;\
        data_a->temps_cells_6.t1 = t1_;\
        data_a->temps_cells_6.t2 = t2_;\
        data_a->temps_cells_6.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_6(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_6, .DLC=DLC_CELL_INFO_6, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_6.delta = delta_;\
        data_a->cell_info_6.ov = ov_;\
        data_a->cell_info_6.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_6(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_6, .DLC=DLC_POWER_LIM_6, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_6.disch_lim = disch_lim_;\
        data_a->power_lim_6.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_7(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_7, .DLC=DLC_SOC_CELLS_7, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_7.idx = idx_;\
        data_a->soc_cells_7.soc1 = soc1_;\
        data_a->soc_cells_7.soc2 = soc2_;\
        data_a->soc_cells_7.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_7(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_7, .DLC=DLC_VOLTS_CELLS_7, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_7.idx = idx_;\
        data_a->volts_cells_7.v1 = v1_;\
        data_a->volts_cells_7.v2 = v2_;\
        data_a->volts_cells_7.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_7(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_7, .DLC=DLC_PACK_INFO_7, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_7.volts = volts_;\
        data_a->pack_info_7.error = error_;\
        data_a->pack_info_7.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_7(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_7, .DLC=DLC_TEMPS_CELLS_7, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_7.idx = idx_;\
        data_a->temps_cells_7.t1 = t1_;\
        data_a->temps_cells_7.t2 = t2_;\
        data_a->temps_cells_7.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_7(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_7, .DLC=DLC_CELL_INFO_7, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_7.delta = delta_;\
        data_a->cell_info_7.ov = ov_;\
        data_a->cell_info_7.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_7(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_7, .DLC=DLC_POWER_LIM_7, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_7.disch_lim = disch_lim_;\
        data_a->power_lim_7.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_SOC_CELLS_8(queue, idx_, soc1_, soc2_, soc3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SOC_CELLS_8, .DLC=DLC_SOC_CELLS_8, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->soc_cells_8.idx = idx_;\
        data_a->soc_cells_8.soc1 = soc1_;\
        data_a->soc_cells_8.soc2 = soc2_;\
        data_a->soc_cells_8.soc3 = soc3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_VOLTS_CELLS_8(queue, idx_, v1_, v2_, v3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_VOLTS_CELLS_8, .DLC=DLC_VOLTS_CELLS_8, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->volts_cells_8.idx = idx_;\
        data_a->volts_cells_8.v1 = v1_;\
        data_a->volts_cells_8.v2 = v2_;\
        data_a->volts_cells_8.v3 = v3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_PACK_INFO_8(queue, volts_, error_, bal_flags_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_INFO_8, .DLC=DLC_PACK_INFO_8, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_info_8.volts = volts_;\
        data_a->pack_info_8.error = error_;\
        data_a->pack_info_8.bal_flags = bal_flags_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_TEMPS_CELLS_8(queue, idx_, t1_, t2_, t3_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_TEMPS_CELLS_8, .DLC=DLC_TEMPS_CELLS_8, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->temps_cells_8.idx = idx_;\
        data_a->temps_cells_8.t1 = t1_;\
        data_a->temps_cells_8.t2 = t2_;\
        data_a->temps_cells_8.t3 = t3_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_CELL_INFO_8(queue, delta_, ov_, uv_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_CELL_INFO_8, .DLC=DLC_CELL_INFO_8, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->cell_info_8.delta = delta_;\
        data_a->cell_info_8.ov = ov_;\
        data_a->cell_info_8.uv = uv_;\
        qSendToBack(&queue, &msg);\
    } while(0)
#define SEND_POWER_LIM_8(queue, disch_lim_, chg_lim_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_POWER_LIM_8, .DLC=DLC_POWER_LIM_8, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->power_lim_8.disch_lim = disch_lim_;\
        data_a->power_lim_8.chg_lim = chg_lim_;\
        qSendToBack(&queue, &msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 5 / 2 // 5 / 2 would be 250% of period
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