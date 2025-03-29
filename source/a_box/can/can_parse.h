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
#include "common/phal_F4_F7/can/can.h"
#include "common/daq/can_parse_base.h"

// Make this match the node name within the can_config.json
#define NODE_NAME "a_box"

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_CAN2_PRECHARGE_HB 0xc001944
#define ID_CAN2_ELCON_CHARGER_COMMAND 0x1806e5f4
#define ID_CAN2_NUM_THERM_BAD 0x100080c4
#define ID_CAN2_PACK_CHARGE_STATUS 0x14008084
#define ID_CAN2_MAX_CELL_TEMP 0xc04e604
#define ID_CAN2_MOD_CELL_TEMP_AVG_A_B_C 0x14013484
#define ID_CAN2_MOD_CELL_TEMP_AVG_D_E 0x140134c4
#define ID_CAN2_MOD_CELL_TEMP_MAX_A_B_C 0x14008104
#define ID_CAN2_MOD_CELL_TEMP_MAX_D_E 0x14008144
#define ID_CAN2_MOD_CELL_TEMP_MIN_A_B_C 0x14008204
#define ID_CAN2_MOD_CELL_TEMP_MIN_D_E 0x14008244
#define ID_CAN2_RAW_CELL_TEMP_MODULE1 0x14013184
#define ID_CAN2_RAW_CELL_TEMP_MODULE2 0x14008384
#define ID_CAN2_RAW_CELL_TEMP_MODULE3 0x140083c4
#define ID_CAN2_RAW_CELL_TEMP_MODULE4 0x14008404
#define ID_CAN2_RAW_CELL_TEMP_MODULE5 0x14008444
#define ID_CAN2_A_BOX_CAN_STATS 0x10016304
#define ID_CAN2_I_SENSE 0x10016444
#define ID_DAQ_RESPONSE_A_BOX_CCAN 0x17ffffc4
#define ID_PRECHARGE_HB 0xc001944
#define ID_ELCON_CHARGER_COMMAND 0x1806e5f4
#define ID_NUM_THERM_BAD 0x100080c4
#define ID_PACK_CHARGE_STATUS 0x14008084
#define ID_MAX_CELL_TEMP 0xc04e604
#define ID_MOD_CELL_TEMP_AVG_A_B_C 0x14013484
#define ID_MOD_CELL_TEMP_AVG_D_E 0x140134c4
#define ID_MOD_CELL_TEMP_MAX_A_B_C 0x14008104
#define ID_MOD_CELL_TEMP_MAX_D_E 0x14008144
#define ID_MOD_CELL_TEMP_MIN_A_B_C 0x14008204
#define ID_MOD_CELL_TEMP_MIN_D_E 0x14008244
#define ID_RAW_CELL_TEMP_MODULE1 0x14013184
#define ID_RAW_CELL_TEMP_MODULE2 0x14008384
#define ID_RAW_CELL_TEMP_MODULE3 0x140083c4
#define ID_RAW_CELL_TEMP_MODULE4 0x14008404
#define ID_RAW_CELL_TEMP_MODULE5 0x14008444
#define ID_A_BOX_CAN_STATS 0x10016304
#define ID_I_SENSE 0x10016444
#define ID_FAULT_SYNC_A_BOX 0x8ca44
#define ID_DAQ_RESPONSE_A_BOX_VCAN 0x17ffffc4
#define ID_ELCON_CHARGER_STATUS 0x18ff50e5
#define ID_ORION_INFO_CHARGER 0x140006b8
#define ID_ORION_CURRENTS_VOLTS_CHARGER 0x140006f8
#define ID_ORION_ERRORS_CHARGER 0xc000738
#define ID_DAQ_COMMAND_A_BOX_CCAN 0x14000132
#define ID_ORION_INFO 0x140006b8
#define ID_ORION_CURRENTS_VOLTS 0x140006f8
#define ID_ORION_ERRORS 0xc000738
#define ID_A_BOX_BL_CMD 0x409c4fe
#define ID_FAULT_SYNC_PDU 0x8cb1f
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_FAULT_SYNC_DASHBOARD 0x8cac5
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8cab7
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_A_BOX_VCAN 0x14000132
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_CAN2_PRECHARGE_HB 2
#define DLC_CAN2_ELCON_CHARGER_COMMAND 5
#define DLC_CAN2_NUM_THERM_BAD 5
#define DLC_CAN2_PACK_CHARGE_STATUS 7
#define DLC_CAN2_MAX_CELL_TEMP 2
#define DLC_CAN2_MOD_CELL_TEMP_AVG_A_B_C 6
#define DLC_CAN2_MOD_CELL_TEMP_AVG_D_E 4
#define DLC_CAN2_MOD_CELL_TEMP_MAX_A_B_C 6
#define DLC_CAN2_MOD_CELL_TEMP_MAX_D_E 4
#define DLC_CAN2_MOD_CELL_TEMP_MIN_A_B_C 6
#define DLC_CAN2_MOD_CELL_TEMP_MIN_D_E 4
#define DLC_CAN2_RAW_CELL_TEMP_MODULE1 5
#define DLC_CAN2_RAW_CELL_TEMP_MODULE2 5
#define DLC_CAN2_RAW_CELL_TEMP_MODULE3 5
#define DLC_CAN2_RAW_CELL_TEMP_MODULE4 5
#define DLC_CAN2_RAW_CELL_TEMP_MODULE5 5
#define DLC_CAN2_A_BOX_CAN_STATS 7
#define DLC_CAN2_I_SENSE 4
#define DLC_DAQ_RESPONSE_A_BOX_CCAN 8
#define DLC_PRECHARGE_HB 2
#define DLC_ELCON_CHARGER_COMMAND 5
#define DLC_NUM_THERM_BAD 5
#define DLC_PACK_CHARGE_STATUS 7
#define DLC_MAX_CELL_TEMP 2
#define DLC_MOD_CELL_TEMP_AVG_A_B_C 6
#define DLC_MOD_CELL_TEMP_AVG_D_E 4
#define DLC_MOD_CELL_TEMP_MAX_A_B_C 6
#define DLC_MOD_CELL_TEMP_MAX_D_E 4
#define DLC_MOD_CELL_TEMP_MIN_A_B_C 6
#define DLC_MOD_CELL_TEMP_MIN_D_E 4
#define DLC_RAW_CELL_TEMP_MODULE1 5
#define DLC_RAW_CELL_TEMP_MODULE2 5
#define DLC_RAW_CELL_TEMP_MODULE3 5
#define DLC_RAW_CELL_TEMP_MODULE4 5
#define DLC_RAW_CELL_TEMP_MODULE5 5
#define DLC_A_BOX_CAN_STATS 7
#define DLC_I_SENSE 4
#define DLC_FAULT_SYNC_A_BOX 3
#define DLC_DAQ_RESPONSE_A_BOX_VCAN 8
#define DLC_ELCON_CHARGER_STATUS 5
#define DLC_ORION_INFO_CHARGER 7
#define DLC_ORION_CURRENTS_VOLTS_CHARGER 4
#define DLC_ORION_ERRORS_CHARGER 4
#define DLC_DAQ_COMMAND_A_BOX_CCAN 8
#define DLC_ORION_INFO 7
#define DLC_ORION_CURRENTS_VOLTS 4
#define DLC_ORION_ERRORS 4
#define DLC_A_BOX_BL_CMD 5
#define DLC_FAULT_SYNC_PDU 3
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_A_BOX_VCAN 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_CAN2_PRECHARGE_HB(IMD_, BMS_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_PRECHARGE_HB, .DLC=DLC_CAN2_PRECHARGE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_precharge_hb.IMD = IMD_;\
        data_a->can2_precharge_hb.BMS = BMS_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_ELCON_CHARGER_COMMAND(voltage_limit_, current_limit_, charge_disable_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_ELCON_CHARGER_COMMAND, .DLC=DLC_CAN2_ELCON_CHARGER_COMMAND, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_elcon_charger_command.voltage_limit = voltage_limit_;\
        data_a->can2_elcon_charger_command.current_limit = current_limit_;\
        data_a->can2_elcon_charger_command.charge_disable = charge_disable_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_NUM_THERM_BAD(A_left_, A_right_, B_left_, B_right_, C_left_, C_right_, D_left_, D_right_, E_left_, E_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_NUM_THERM_BAD, .DLC=DLC_CAN2_NUM_THERM_BAD, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_num_therm_bad.A_left = A_left_;\
        data_a->can2_num_therm_bad.A_right = A_right_;\
        data_a->can2_num_therm_bad.B_left = B_left_;\
        data_a->can2_num_therm_bad.B_right = B_right_;\
        data_a->can2_num_therm_bad.C_left = C_left_;\
        data_a->can2_num_therm_bad.C_right = C_right_;\
        data_a->can2_num_therm_bad.D_left = D_left_;\
        data_a->can2_num_therm_bad.D_right = D_right_;\
        data_a->can2_num_therm_bad.E_left = E_left_;\
        data_a->can2_num_therm_bad.E_right = E_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_PACK_CHARGE_STATUS(power_, charge_enable_, voltage_, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_PACK_CHARGE_STATUS, .DLC=DLC_CAN2_PACK_CHARGE_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_pack_charge_status.power = power_;\
        data_a->can2_pack_charge_status.charge_enable = charge_enable_;\
        data_a->can2_pack_charge_status.voltage = voltage_;\
        data_a->can2_pack_charge_status.current = current_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MAX_CELL_TEMP(max_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MAX_CELL_TEMP, .DLC=DLC_CAN2_MAX_CELL_TEMP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_max_cell_temp.max_temp = max_temp_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MOD_CELL_TEMP_AVG_A_B_C(temp_A_, temp_B_, temp_C_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MOD_CELL_TEMP_AVG_A_B_C, .DLC=DLC_CAN2_MOD_CELL_TEMP_AVG_A_B_C, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_mod_cell_temp_avg_a_b_c.temp_A = temp_A_;\
        data_a->can2_mod_cell_temp_avg_a_b_c.temp_B = temp_B_;\
        data_a->can2_mod_cell_temp_avg_a_b_c.temp_C = temp_C_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MOD_CELL_TEMP_AVG_D_E(temp_D_, temp_E_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MOD_CELL_TEMP_AVG_D_E, .DLC=DLC_CAN2_MOD_CELL_TEMP_AVG_D_E, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_mod_cell_temp_avg_d_e.temp_D = temp_D_;\
        data_a->can2_mod_cell_temp_avg_d_e.temp_E = temp_E_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MOD_CELL_TEMP_MAX_A_B_C(temp_A_, temp_B_, temp_C_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MOD_CELL_TEMP_MAX_A_B_C, .DLC=DLC_CAN2_MOD_CELL_TEMP_MAX_A_B_C, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_mod_cell_temp_max_a_b_c.temp_A = temp_A_;\
        data_a->can2_mod_cell_temp_max_a_b_c.temp_B = temp_B_;\
        data_a->can2_mod_cell_temp_max_a_b_c.temp_C = temp_C_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MOD_CELL_TEMP_MAX_D_E(temp_D_, temp_E_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MOD_CELL_TEMP_MAX_D_E, .DLC=DLC_CAN2_MOD_CELL_TEMP_MAX_D_E, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_mod_cell_temp_max_d_e.temp_D = temp_D_;\
        data_a->can2_mod_cell_temp_max_d_e.temp_E = temp_E_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MOD_CELL_TEMP_MIN_A_B_C(temp_A_, temp_B_, temp_C_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MOD_CELL_TEMP_MIN_A_B_C, .DLC=DLC_CAN2_MOD_CELL_TEMP_MIN_A_B_C, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_mod_cell_temp_min_a_b_c.temp_A = temp_A_;\
        data_a->can2_mod_cell_temp_min_a_b_c.temp_B = temp_B_;\
        data_a->can2_mod_cell_temp_min_a_b_c.temp_C = temp_C_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_MOD_CELL_TEMP_MIN_D_E(temp_D_, temp_E_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_MOD_CELL_TEMP_MIN_D_E, .DLC=DLC_CAN2_MOD_CELL_TEMP_MIN_D_E, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_mod_cell_temp_min_d_e.temp_D = temp_D_;\
        data_a->can2_mod_cell_temp_min_d_e.temp_E = temp_E_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_RAW_CELL_TEMP_MODULE1(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_RAW_CELL_TEMP_MODULE1, .DLC=DLC_CAN2_RAW_CELL_TEMP_MODULE1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_raw_cell_temp_module1.index = index_;\
        data_a->can2_raw_cell_temp_module1.temp_left = temp_left_;\
        data_a->can2_raw_cell_temp_module1.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_RAW_CELL_TEMP_MODULE2(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_RAW_CELL_TEMP_MODULE2, .DLC=DLC_CAN2_RAW_CELL_TEMP_MODULE2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_raw_cell_temp_module2.index = index_;\
        data_a->can2_raw_cell_temp_module2.temp_left = temp_left_;\
        data_a->can2_raw_cell_temp_module2.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_RAW_CELL_TEMP_MODULE3(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_RAW_CELL_TEMP_MODULE3, .DLC=DLC_CAN2_RAW_CELL_TEMP_MODULE3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_raw_cell_temp_module3.index = index_;\
        data_a->can2_raw_cell_temp_module3.temp_left = temp_left_;\
        data_a->can2_raw_cell_temp_module3.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_RAW_CELL_TEMP_MODULE4(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_RAW_CELL_TEMP_MODULE4, .DLC=DLC_CAN2_RAW_CELL_TEMP_MODULE4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_raw_cell_temp_module4.index = index_;\
        data_a->can2_raw_cell_temp_module4.temp_left = temp_left_;\
        data_a->can2_raw_cell_temp_module4.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_RAW_CELL_TEMP_MODULE5(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_RAW_CELL_TEMP_MODULE5, .DLC=DLC_CAN2_RAW_CELL_TEMP_MODULE5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_raw_cell_temp_module5.index = index_;\
        data_a->can2_raw_cell_temp_module5.temp_left = temp_left_;\
        data_a->can2_raw_cell_temp_module5.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_A_BOX_CAN_STATS(can1_tx_queue_overflow_, can2_tx_queue_overflow_, can1_tx_fail_, can2_tx_fail_, can_rx_queue_overflow_, can1_rx_overrun_, can2_rx_overrun_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_A_BOX_CAN_STATS, .DLC=DLC_CAN2_A_BOX_CAN_STATS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_a_box_can_stats.can1_tx_queue_overflow = can1_tx_queue_overflow_;\
        data_a->can2_a_box_can_stats.can2_tx_queue_overflow = can2_tx_queue_overflow_;\
        data_a->can2_a_box_can_stats.can1_tx_fail = can1_tx_fail_;\
        data_a->can2_a_box_can_stats.can2_tx_fail = can2_tx_fail_;\
        data_a->can2_a_box_can_stats.can_rx_queue_overflow = can_rx_queue_overflow_;\
        data_a->can2_a_box_can_stats.can1_rx_overrun = can1_rx_overrun_;\
        data_a->can2_a_box_can_stats.can2_rx_overrun = can2_rx_overrun_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_CAN2_I_SENSE(current_channel_1_, current_channel_2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_CAN2_I_SENSE, .DLC=DLC_CAN2_I_SENSE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->can2_i_sense.current_channel_1 = current_channel_1_;\
        data_a->can2_i_sense.current_channel_2 = current_channel_2_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_A_BOX_CCAN(daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_DAQ_RESPONSE_A_BOX_CCAN, .DLC=DLC_DAQ_RESPONSE_A_BOX_CCAN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_A_BOX_CCAN.daq_response = daq_response_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_PRECHARGE_HB(IMD_, BMS_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PRECHARGE_HB, .DLC=DLC_PRECHARGE_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->precharge_hb.IMD = IMD_;\
        data_a->precharge_hb.BMS = BMS_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_ELCON_CHARGER_COMMAND(voltage_limit_, current_limit_, charge_disable_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_ELCON_CHARGER_COMMAND, .DLC=DLC_ELCON_CHARGER_COMMAND, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->elcon_charger_command.voltage_limit = voltage_limit_;\
        data_a->elcon_charger_command.current_limit = current_limit_;\
        data_a->elcon_charger_command.charge_disable = charge_disable_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_NUM_THERM_BAD(A_left_, A_right_, B_left_, B_right_, C_left_, C_right_, D_left_, D_right_, E_left_, E_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_NUM_THERM_BAD, .DLC=DLC_NUM_THERM_BAD, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->num_therm_bad.A_left = A_left_;\
        data_a->num_therm_bad.A_right = A_right_;\
        data_a->num_therm_bad.B_left = B_left_;\
        data_a->num_therm_bad.B_right = B_right_;\
        data_a->num_therm_bad.C_left = C_left_;\
        data_a->num_therm_bad.C_right = C_right_;\
        data_a->num_therm_bad.D_left = D_left_;\
        data_a->num_therm_bad.D_right = D_right_;\
        data_a->num_therm_bad.E_left = E_left_;\
        data_a->num_therm_bad.E_right = E_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_PACK_CHARGE_STATUS(power_, charge_enable_, voltage_, current_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_PACK_CHARGE_STATUS, .DLC=DLC_PACK_CHARGE_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->pack_charge_status.power = power_;\
        data_a->pack_charge_status.charge_enable = charge_enable_;\
        data_a->pack_charge_status.voltage = voltage_;\
        data_a->pack_charge_status.current = current_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MAX_CELL_TEMP(max_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAX_CELL_TEMP, .DLC=DLC_MAX_CELL_TEMP, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->max_cell_temp.max_temp = max_temp_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_AVG_A_B_C(temp_A_, temp_B_, temp_C_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_AVG_A_B_C, .DLC=DLC_MOD_CELL_TEMP_AVG_A_B_C, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_avg_a_b_c.temp_A = temp_A_;\
        data_a->mod_cell_temp_avg_a_b_c.temp_B = temp_B_;\
        data_a->mod_cell_temp_avg_a_b_c.temp_C = temp_C_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_AVG_D_E(temp_D_, temp_E_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_AVG_D_E, .DLC=DLC_MOD_CELL_TEMP_AVG_D_E, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_avg_d_e.temp_D = temp_D_;\
        data_a->mod_cell_temp_avg_d_e.temp_E = temp_E_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_MAX_A_B_C(temp_A_, temp_B_, temp_C_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_MAX_A_B_C, .DLC=DLC_MOD_CELL_TEMP_MAX_A_B_C, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_max_a_b_c.temp_A = temp_A_;\
        data_a->mod_cell_temp_max_a_b_c.temp_B = temp_B_;\
        data_a->mod_cell_temp_max_a_b_c.temp_C = temp_C_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_MAX_D_E(temp_D_, temp_E_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_MAX_D_E, .DLC=DLC_MOD_CELL_TEMP_MAX_D_E, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_max_d_e.temp_D = temp_D_;\
        data_a->mod_cell_temp_max_d_e.temp_E = temp_E_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_MIN_A_B_C(temp_A_, temp_B_, temp_C_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_MIN_A_B_C, .DLC=DLC_MOD_CELL_TEMP_MIN_A_B_C, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_min_a_b_c.temp_A = temp_A_;\
        data_a->mod_cell_temp_min_a_b_c.temp_B = temp_B_;\
        data_a->mod_cell_temp_min_a_b_c.temp_C = temp_C_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MOD_CELL_TEMP_MIN_D_E(temp_D_, temp_E_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MOD_CELL_TEMP_MIN_D_E, .DLC=DLC_MOD_CELL_TEMP_MIN_D_E, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mod_cell_temp_min_d_e.temp_D = temp_D_;\
        data_a->mod_cell_temp_min_d_e.temp_E = temp_E_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_RAW_CELL_TEMP_MODULE1(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_CELL_TEMP_MODULE1, .DLC=DLC_RAW_CELL_TEMP_MODULE1, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_cell_temp_module1.index = index_;\
        data_a->raw_cell_temp_module1.temp_left = temp_left_;\
        data_a->raw_cell_temp_module1.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_RAW_CELL_TEMP_MODULE2(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_CELL_TEMP_MODULE2, .DLC=DLC_RAW_CELL_TEMP_MODULE2, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_cell_temp_module2.index = index_;\
        data_a->raw_cell_temp_module2.temp_left = temp_left_;\
        data_a->raw_cell_temp_module2.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_RAW_CELL_TEMP_MODULE3(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_CELL_TEMP_MODULE3, .DLC=DLC_RAW_CELL_TEMP_MODULE3, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_cell_temp_module3.index = index_;\
        data_a->raw_cell_temp_module3.temp_left = temp_left_;\
        data_a->raw_cell_temp_module3.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_RAW_CELL_TEMP_MODULE4(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_CELL_TEMP_MODULE4, .DLC=DLC_RAW_CELL_TEMP_MODULE4, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_cell_temp_module4.index = index_;\
        data_a->raw_cell_temp_module4.temp_left = temp_left_;\
        data_a->raw_cell_temp_module4.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_RAW_CELL_TEMP_MODULE5(index_, temp_left_, temp_right_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_CELL_TEMP_MODULE5, .DLC=DLC_RAW_CELL_TEMP_MODULE5, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_cell_temp_module5.index = index_;\
        data_a->raw_cell_temp_module5.temp_left = temp_left_;\
        data_a->raw_cell_temp_module5.temp_right = temp_right_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_A_BOX_CAN_STATS(can1_tx_queue_overflow_, can2_tx_queue_overflow_, can1_tx_fail_, can2_tx_fail_, can_rx_queue_overflow_, can1_rx_overrun_, can2_rx_overrun_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_A_BOX_CAN_STATS, .DLC=DLC_A_BOX_CAN_STATS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->a_box_can_stats.can1_tx_queue_overflow = can1_tx_queue_overflow_;\
        data_a->a_box_can_stats.can2_tx_queue_overflow = can2_tx_queue_overflow_;\
        data_a->a_box_can_stats.can1_tx_fail = can1_tx_fail_;\
        data_a->a_box_can_stats.can2_tx_fail = can2_tx_fail_;\
        data_a->a_box_can_stats.can_rx_queue_overflow = can_rx_queue_overflow_;\
        data_a->a_box_can_stats.can1_rx_overrun = can1_rx_overrun_;\
        data_a->a_box_can_stats.can2_rx_overrun = can2_rx_overrun_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_I_SENSE(current_channel_1_, current_channel_2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_I_SENSE, .DLC=DLC_I_SENSE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->i_sense.current_channel_1 = current_channel_1_;\
        data_a->i_sense.current_channel_2 = current_channel_2_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_FAULT_SYNC_A_BOX(idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_A_BOX, .DLC=DLC_FAULT_SYNC_A_BOX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_a_box.idx = idx_;\
        data_a->fault_sync_a_box.latched = latched_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_A_BOX_VCAN(daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_A_BOX_VCAN, .DLC=DLC_DAQ_RESPONSE_A_BOX_VCAN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_A_BOX_VCAN.daq_response = daq_response_;\
        canTxSendToBack(&msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 5 / 2 // 5 / 2 would be 250% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_ELCON_CHARGER_STATUS 2000
#define UP_ORION_INFO_CHARGER 32
#define UP_ORION_CURRENTS_VOLTS_CHARGER 32
#define UP_ORION_ERRORS_CHARGER 1000
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
        uint64_t IMD: 8;
        uint64_t BMS: 8;
    } can2_precharge_hb;
    struct {
        uint64_t voltage_limit: 16;
        uint64_t current_limit: 16;
        uint64_t charge_disable: 1;
    } can2_elcon_charger_command;
    struct {
        uint64_t A_left: 4;
        uint64_t A_right: 4;
        uint64_t B_left: 4;
        uint64_t B_right: 4;
        uint64_t C_left: 4;
        uint64_t C_right: 4;
        uint64_t D_left: 4;
        uint64_t D_right: 4;
        uint64_t E_left: 4;
        uint64_t E_right: 4;
    } can2_num_therm_bad;
    struct {
        uint64_t power: 16;
        uint64_t charge_enable: 1;
        uint64_t voltage: 16;
        uint64_t current: 16;
    } can2_pack_charge_status;
    struct {
        uint64_t max_temp: 16;
    } can2_max_cell_temp;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
    } can2_mod_cell_temp_avg_a_b_c;
    struct {
        uint64_t temp_D: 16;
        uint64_t temp_E: 16;
    } can2_mod_cell_temp_avg_d_e;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
    } can2_mod_cell_temp_max_a_b_c;
    struct {
        uint64_t temp_D: 16;
        uint64_t temp_E: 16;
    } can2_mod_cell_temp_max_d_e;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
    } can2_mod_cell_temp_min_a_b_c;
    struct {
        uint64_t temp_D: 16;
        uint64_t temp_E: 16;
    } can2_mod_cell_temp_min_d_e;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } can2_raw_cell_temp_module1;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } can2_raw_cell_temp_module2;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } can2_raw_cell_temp_module3;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } can2_raw_cell_temp_module4;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } can2_raw_cell_temp_module5;
    struct {
        uint64_t can1_tx_queue_overflow: 8;
        uint64_t can2_tx_queue_overflow: 8;
        uint64_t can1_tx_fail: 8;
        uint64_t can2_tx_fail: 8;
        uint64_t can_rx_queue_overflow: 8;
        uint64_t can1_rx_overrun: 8;
        uint64_t can2_rx_overrun: 8;
    } can2_a_box_can_stats;
    struct {
        uint64_t current_channel_1: 16;
        uint64_t current_channel_2: 16;
    } can2_i_sense;
    struct {
        uint64_t daq_response: 64;
    } daq_response_A_BOX_CCAN;
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
        uint64_t A_left: 4;
        uint64_t A_right: 4;
        uint64_t B_left: 4;
        uint64_t B_right: 4;
        uint64_t C_left: 4;
        uint64_t C_right: 4;
        uint64_t D_left: 4;
        uint64_t D_right: 4;
        uint64_t E_left: 4;
        uint64_t E_right: 4;
    } num_therm_bad;
    struct {
        uint64_t power: 16;
        uint64_t charge_enable: 1;
        uint64_t voltage: 16;
        uint64_t current: 16;
    } pack_charge_status;
    struct {
        uint64_t max_temp: 16;
    } max_cell_temp;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
    } mod_cell_temp_avg_a_b_c;
    struct {
        uint64_t temp_D: 16;
        uint64_t temp_E: 16;
    } mod_cell_temp_avg_d_e;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
    } mod_cell_temp_max_a_b_c;
    struct {
        uint64_t temp_D: 16;
        uint64_t temp_E: 16;
    } mod_cell_temp_max_d_e;
    struct {
        uint64_t temp_A: 16;
        uint64_t temp_B: 16;
        uint64_t temp_C: 16;
    } mod_cell_temp_min_a_b_c;
    struct {
        uint64_t temp_D: 16;
        uint64_t temp_E: 16;
    } mod_cell_temp_min_d_e;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } raw_cell_temp_module1;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } raw_cell_temp_module2;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } raw_cell_temp_module3;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } raw_cell_temp_module4;
    struct {
        uint64_t index: 8;
        uint64_t temp_left: 16;
        uint64_t temp_right: 16;
    } raw_cell_temp_module5;
    struct {
        uint64_t can1_tx_queue_overflow: 8;
        uint64_t can2_tx_queue_overflow: 8;
        uint64_t can1_tx_fail: 8;
        uint64_t can2_tx_fail: 8;
        uint64_t can_rx_queue_overflow: 8;
        uint64_t can1_rx_overrun: 8;
        uint64_t can2_rx_overrun: 8;
    } a_box_can_stats;
    struct {
        uint64_t current_channel_1: 16;
        uint64_t current_channel_2: 16;
    } i_sense;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_a_box;
    struct {
        uint64_t daq_response: 64;
    } daq_response_A_BOX_VCAN;
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
    } orion_info_charger;
    struct {
        uint64_t pack_current: 16;
        uint64_t pack_voltage: 16;
    } orion_currents_volts_charger;
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
    } orion_errors_charger;
    struct {
        uint64_t daq_command: 64;
    } daq_command_A_BOX_CCAN;
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
    } a_box_bl_cmd;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_pdu;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
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
    } daq_command_A_BOX_VCAN;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
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
    } orion_info_charger;
    struct {
        int16_t pack_current;
        uint16_t pack_voltage;
        uint8_t stale;
        uint32_t last_rx;
    } orion_currents_volts_charger;
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
    } orion_errors_charger;
    struct {
        uint64_t daq_command;
    } daq_command_A_BOX_CCAN;
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
    } a_box_bl_cmd;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_pdu;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_main_module;
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
    } daq_command_A_BOX_VCAN;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_A_BOX_CCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void daq_command_A_BOX_VCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void a_box_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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
void initCANParse(void);

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
