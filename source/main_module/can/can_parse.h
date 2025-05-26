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
#define NODE_NAME "Main_Module"


// Used to represent a float as 32 bits
typedef union {
    float f;
    uint32_t u;
} FloatConvert_t;
#define FLOAT_TO_UINT32(float_) (((FloatConvert_t) float_).u)
#define UINT32_TO_FLOAT(uint32_) (((FloatConvert_t) ((uint32_t) uint32_)).f)

// Message ID definitions
/* BEGIN AUTO ID DEFS */
#define ID_MAIN_HB_AMK 0xc001941
#define ID_INVA_SET 0x188
#define ID_INVB_SET 0x189
#define ID_MAIN_HB 0xc001901
#define ID_COOLANT_TEMPS 0x10000881
#define ID_GEARBOX 0x10000901
#define ID_LWS_CONFIG 0x7c0
#define ID_LOAD_SENSOR_READINGS 0x1000fa01
#define ID_SHOCK_REAR 0x1000ff01
#define ID_MCU_STATUS 0x10001981
#define ID_MAIN_MODULE_CAN_STATS 0x10016301
#define ID_NUM_MC_SKIPS 0x10001b81
#define ID_REAR_MC_STATUS 0x10001941
#define ID_REAR_MOTOR_CURRENTS_VOLTS 0x100002c1
#define ID_SDC_STATUS 0xc000381
#define ID_REAR_MOTOR_TEMPS 0x10000301
#define ID_REAR_WHEEL_SPEEDS 0x4000381
#define ID_FAULT_SYNC_MAIN_MODULE 0x8ca01
#define ID_DAQ_RESPONSE_MAIN_MODULE_VCAN 0x17ffffc1
#define ID_INVA_CRIT 0x282
#define ID_INVA_INFO 0x286
#define ID_INVA_TEMPS 0x284
#define ID_INVA_ERR_1 0x288
#define ID_INVA_ERR_2 0x290
#define ID_INVB_CRIT 0x283
#define ID_INVB_INFO 0x287
#define ID_INVB_TEMPS 0x285
#define ID_INVB_ERR_1 0x289
#define ID_INVB_ERR_2 0x291
#define ID_RAW_THROTTLE_BRAKE 0x10000285
#define ID_FILT_THROTTLE_BRAKE 0x4000245
#define ID_START_BUTTON 0x4000005
#define ID_MAX_CELL_TEMP 0xc04e604
#define ID_LWS_STANDARD 0x2b0
#define ID_MAIN_MODULE_BL_CMD 0x409c43e
#define ID_ORION_CURRENTS_VOLTS 0x140006f8
#define ID_VCU_TORQUES_SPEEDS 0x40026b7
#define ID_DRIVE_MODES 0xc002737
#define ID_ORION_INFO 0x140006b8
#define ID_FAULT_SYNC_PDU 0x8cb1f
#define ID_FAULT_SYNC_DASHBOARD 0x8cac5
#define ID_FAULT_SYNC_A_BOX 0x8ca44
#define ID_FAULT_SYNC_TORQUE_VECTOR 0x8cab7
#define ID_FAULT_SYNC_TEST_NODE 0x8cb7f
#define ID_SET_FAULT 0x809c83e
#define ID_RETURN_FAULT_CONTROL 0x809c87e
#define ID_DAQ_COMMAND_MAIN_MODULE_VCAN 0x14000072
/* END AUTO ID DEFS */

// Message DLC definitions
/* BEGIN AUTO DLC DEFS */
#define DLC_MAIN_HB_AMK 2
#define DLC_INVA_SET 8
#define DLC_INVB_SET 8
#define DLC_MAIN_HB 2
#define DLC_COOLANT_TEMPS 4
#define DLC_GEARBOX 2
#define DLC_LWS_CONFIG 2
#define DLC_LOAD_SENSOR_READINGS 8
#define DLC_SHOCK_REAR 4
#define DLC_MCU_STATUS 4
#define DLC_MAIN_MODULE_CAN_STATS 7
#define DLC_NUM_MC_SKIPS 4
#define DLC_REAR_MC_STATUS 6
#define DLC_REAR_MOTOR_CURRENTS_VOLTS 6
#define DLC_SDC_STATUS 2
#define DLC_REAR_MOTOR_TEMPS 6
#define DLC_REAR_WHEEL_SPEEDS 4
#define DLC_FAULT_SYNC_MAIN_MODULE 3
#define DLC_DAQ_RESPONSE_MAIN_MODULE_VCAN 8
#define DLC_INVA_CRIT 8
#define DLC_INVA_INFO 4
#define DLC_INVA_TEMPS 6
#define DLC_INVA_ERR_1 8
#define DLC_INVA_ERR_2 8
#define DLC_INVB_CRIT 8
#define DLC_INVB_INFO 4
#define DLC_INVB_TEMPS 6
#define DLC_INVB_ERR_1 8
#define DLC_INVB_ERR_2 8
#define DLC_RAW_THROTTLE_BRAKE 8
#define DLC_FILT_THROTTLE_BRAKE 3
#define DLC_START_BUTTON 1
#define DLC_MAX_CELL_TEMP 2
#define DLC_LWS_STANDARD 5
#define DLC_MAIN_MODULE_BL_CMD 5
#define DLC_ORION_CURRENTS_VOLTS 4
#define DLC_VCU_TORQUES_SPEEDS 8
#define DLC_DRIVE_MODES 2
#define DLC_ORION_INFO 7
#define DLC_FAULT_SYNC_PDU 3
#define DLC_FAULT_SYNC_DASHBOARD 3
#define DLC_FAULT_SYNC_A_BOX 3
#define DLC_FAULT_SYNC_TORQUE_VECTOR 3
#define DLC_FAULT_SYNC_TEST_NODE 3
#define DLC_SET_FAULT 3
#define DLC_RETURN_FAULT_CONTROL 2
#define DLC_DAQ_COMMAND_MAIN_MODULE_VCAN 8
/* END AUTO DLC DEFS */

// Message sending macros
/* BEGIN AUTO SEND MACROS */
#define SEND_MAIN_HB_AMK(car_state_, precharge_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .ExtId=ID_MAIN_HB_AMK, .DLC=DLC_MAIN_HB_AMK, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_hb_amk.car_state = car_state_;\
        data_a->main_hb_amk.precharge_state = precharge_state_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_INVA_SET(AMK_Control_bReserve_, AMK_Control_bInverterOn_, AMK_Control_bDcOn_, AMK_Control_bEnable_, AMK_Control_bErrorReset_, AMK_Control_bReserve2_, AMK_TorqueSetpoint_, AMK_PositiveTorqueLimit_, AMK_NegativeTorqueLimit_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .StdId=ID_INVA_SET, .DLC=DLC_INVA_SET, .IDE=0};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->INVA_SET.AMK_Control_bReserve = AMK_Control_bReserve_;\
        data_a->INVA_SET.AMK_Control_bInverterOn = AMK_Control_bInverterOn_;\
        data_a->INVA_SET.AMK_Control_bDcOn = AMK_Control_bDcOn_;\
        data_a->INVA_SET.AMK_Control_bEnable = AMK_Control_bEnable_;\
        data_a->INVA_SET.AMK_Control_bErrorReset = AMK_Control_bErrorReset_;\
        data_a->INVA_SET.AMK_Control_bReserve2 = AMK_Control_bReserve2_;\
        data_a->INVA_SET.AMK_TorqueSetpoint = AMK_TorqueSetpoint_;\
        data_a->INVA_SET.AMK_PositiveTorqueLimit = AMK_PositiveTorqueLimit_;\
        data_a->INVA_SET.AMK_NegativeTorqueLimit = AMK_NegativeTorqueLimit_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_INVB_SET(AMK_Control_bReserve_, AMK_Control_bInverterOn_, AMK_Control_bDcOn_, AMK_Control_bEnable_, AMK_Control_bErrorReset_, AMK_Control_bReserve2_, AMK_TorqueSetpoint_, AMK_PositiveTorqueLimit_, AMK_NegativeTorqueLimit_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN2, .StdId=ID_INVB_SET, .DLC=DLC_INVB_SET, .IDE=0};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->INVB_SET.AMK_Control_bReserve = AMK_Control_bReserve_;\
        data_a->INVB_SET.AMK_Control_bInverterOn = AMK_Control_bInverterOn_;\
        data_a->INVB_SET.AMK_Control_bDcOn = AMK_Control_bDcOn_;\
        data_a->INVB_SET.AMK_Control_bEnable = AMK_Control_bEnable_;\
        data_a->INVB_SET.AMK_Control_bErrorReset = AMK_Control_bErrorReset_;\
        data_a->INVB_SET.AMK_Control_bReserve2 = AMK_Control_bReserve2_;\
        data_a->INVB_SET.AMK_TorqueSetpoint = AMK_TorqueSetpoint_;\
        data_a->INVB_SET.AMK_PositiveTorqueLimit = AMK_PositiveTorqueLimit_;\
        data_a->INVB_SET.AMK_NegativeTorqueLimit = AMK_NegativeTorqueLimit_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MAIN_HB(car_state_, precharge_state_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_HB, .DLC=DLC_MAIN_HB, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_hb.car_state = car_state_;\
        data_a->main_hb.precharge_state = precharge_state_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_COOLANT_TEMPS(battery_in_temp_, battery_out_temp_, drivetrain_in_temp_, drivetrain_out_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_COOLANT_TEMPS, .DLC=DLC_COOLANT_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->coolant_temps.battery_in_temp = battery_in_temp_;\
        data_a->coolant_temps.battery_out_temp = battery_out_temp_;\
        data_a->coolant_temps.drivetrain_in_temp = drivetrain_in_temp_;\
        data_a->coolant_temps.drivetrain_out_temp = drivetrain_out_temp_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_GEARBOX(l_temp_, r_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_GEARBOX, .DLC=DLC_GEARBOX, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->gearbox.l_temp = l_temp_;\
        data_a->gearbox.r_temp = r_temp_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_LWS_CONFIG(CCW_, Reserved_1_, Reserved_2_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .StdId=ID_LWS_CONFIG, .DLC=DLC_LWS_CONFIG, .IDE=0};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->LWS_Config.CCW = CCW_;\
        data_a->LWS_Config.Reserved_1 = Reserved_1_;\
        data_a->LWS_Config.Reserved_2 = Reserved_2_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_LOAD_SENSOR_READINGS(left_load_sensor_, right_load_sensor_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_LOAD_SENSOR_READINGS, .DLC=DLC_LOAD_SENSOR_READINGS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->load_sensor_readings.left_load_sensor = FLOAT_TO_UINT32(left_load_sensor_);\
        data_a->load_sensor_readings.right_load_sensor = FLOAT_TO_UINT32(right_load_sensor_);\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_SHOCK_REAR(left_shock_, right_shock_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SHOCK_REAR, .DLC=DLC_SHOCK_REAR, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->shock_rear.left_shock = left_shock_;\
        data_a->shock_rear.right_shock = right_shock_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MCU_STATUS(sched_skips_, foreground_use_, background_use_, sched_error_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MCU_STATUS, .DLC=DLC_MCU_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->mcu_status.sched_skips = sched_skips_;\
        data_a->mcu_status.foreground_use = foreground_use_;\
        data_a->mcu_status.background_use = background_use_;\
        data_a->mcu_status.sched_error = sched_error_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_MAIN_MODULE_CAN_STATS(can1_tx_queue_overflow_, can2_tx_queue_overflow_, can1_tx_fail_, can2_tx_fail_, can_rx_queue_overflow_, can1_rx_overrun_, can2_rx_overrun_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_MAIN_MODULE_CAN_STATS, .DLC=DLC_MAIN_MODULE_CAN_STATS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->main_module_can_stats.can1_tx_queue_overflow = can1_tx_queue_overflow_;\
        data_a->main_module_can_stats.can2_tx_queue_overflow = can2_tx_queue_overflow_;\
        data_a->main_module_can_stats.can1_tx_fail = can1_tx_fail_;\
        data_a->main_module_can_stats.can2_tx_fail = can2_tx_fail_;\
        data_a->main_module_can_stats.can_rx_queue_overflow = can_rx_queue_overflow_;\
        data_a->main_module_can_stats.can1_rx_overrun = can1_rx_overrun_;\
        data_a->main_module_can_stats.can2_rx_overrun = can2_rx_overrun_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_NUM_MC_SKIPS(noise_r_, noise_l_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_NUM_MC_SKIPS, .DLC=DLC_NUM_MC_SKIPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->num_mc_skips.noise_r = noise_r_;\
        data_a->num_mc_skips.noise_l = noise_l_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_REAR_MC_STATUS(rear_left_motor_, rear_left_motor_link_, rear_left_last_link_error_, rear_right_motor_, rear_right_motor_link_, rear_right_last_link_error_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MC_STATUS, .DLC=DLC_REAR_MC_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_mc_status.rear_left_motor = rear_left_motor_;\
        data_a->rear_mc_status.rear_left_motor_link = rear_left_motor_link_;\
        data_a->rear_mc_status.rear_left_last_link_error = rear_left_last_link_error_;\
        data_a->rear_mc_status.rear_right_motor = rear_right_motor_;\
        data_a->rear_mc_status.rear_right_motor_link = rear_right_motor_link_;\
        data_a->rear_mc_status.rear_right_last_link_error = rear_right_last_link_error_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_REAR_MOTOR_CURRENTS_VOLTS(left_current_, right_current_, right_voltage_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_CURRENTS_VOLTS, .DLC=DLC_REAR_MOTOR_CURRENTS_VOLTS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_currents_volts.left_current = left_current_;\
        data_a->rear_motor_currents_volts.right_current = right_current_;\
        data_a->rear_motor_currents_volts.right_voltage = right_voltage_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_SDC_STATUS(main_, c_estop_, inertia_, BOTS_, BSPD_, BMS_, IMD_, r_estop_, l_estop_, HVD_, emeter_, hub_, TSMS_, pchg_out_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_SDC_STATUS, .DLC=DLC_SDC_STATUS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->sdc_status.main = main_;\
        data_a->sdc_status.c_estop = c_estop_;\
        data_a->sdc_status.inertia = inertia_;\
        data_a->sdc_status.BOTS = BOTS_;\
        data_a->sdc_status.BSPD = BSPD_;\
        data_a->sdc_status.BMS = BMS_;\
        data_a->sdc_status.IMD = IMD_;\
        data_a->sdc_status.r_estop = r_estop_;\
        data_a->sdc_status.l_estop = l_estop_;\
        data_a->sdc_status.HVD = HVD_;\
        data_a->sdc_status.emeter = emeter_;\
        data_a->sdc_status.hub = hub_;\
        data_a->sdc_status.TSMS = TSMS_;\
        data_a->sdc_status.pchg_out = pchg_out_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_REAR_MOTOR_TEMPS(left_mot_temp_, right_mot_temp_, left_inv_temp_, right_inv_temp_, left_igbt_temp_, right_igbt_temp_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_MOTOR_TEMPS, .DLC=DLC_REAR_MOTOR_TEMPS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_motor_temps.left_mot_temp = left_mot_temp_;\
        data_a->rear_motor_temps.right_mot_temp = right_mot_temp_;\
        data_a->rear_motor_temps.left_inv_temp = left_inv_temp_;\
        data_a->rear_motor_temps.right_inv_temp = right_inv_temp_;\
        data_a->rear_motor_temps.left_igbt_temp = left_igbt_temp_;\
        data_a->rear_motor_temps.right_igbt_temp = right_igbt_temp_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_REAR_WHEEL_SPEEDS(left_speed_sensor_, right_speed_sensor_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_REAR_WHEEL_SPEEDS, .DLC=DLC_REAR_WHEEL_SPEEDS, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->rear_wheel_speeds.left_speed_sensor = left_speed_sensor_;\
        data_a->rear_wheel_speeds.right_speed_sensor = right_speed_sensor_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_FAULT_SYNC_MAIN_MODULE(idx_, latched_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_FAULT_SYNC_MAIN_MODULE, .DLC=DLC_FAULT_SYNC_MAIN_MODULE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->fault_sync_main_module.idx = idx_;\
        data_a->fault_sync_main_module.latched = latched_;\
        canTxSendToBack(&msg);\
    } while(0)
#define SEND_DAQ_RESPONSE_MAIN_MODULE_VCAN(daq_response_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_DAQ_RESPONSE_MAIN_MODULE_VCAN, .DLC=DLC_DAQ_RESPONSE_MAIN_MODULE_VCAN, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->daq_response_MAIN_MODULE_VCAN.daq_response = daq_response_;\
        canTxSendToBack(&msg);\
    } while(0)
/* END AUTO SEND MACROS */

// Stale Checking
#define STALE_THRESH 30 / 2 // 5 / 2 would be 250% of period
/* BEGIN AUTO UP DEFS (Update Period)*/
#define UP_INVA_CRIT 15
#define UP_INVA_INFO 500
#define UP_INVA_TEMPS 500
#define UP_INVA_SET 15
#define UP_INVA_ERR_1 2000
#define UP_INVA_ERR_2 2000
#define UP_INVB_CRIT 15
#define UP_INVB_INFO 500
#define UP_INVB_TEMPS 500
#define UP_INVB_SET 15
#define UP_INVB_ERR_1 2000
#define UP_INVB_ERR_2 2000
#define UP_RAW_THROTTLE_BRAKE 15
#define UP_FILT_THROTTLE_BRAKE 15
#define UP_MAX_CELL_TEMP 500
#define UP_LWS_STANDARD 15
#define UP_ORION_CURRENTS_VOLTS 32
#define UP_VCU_TORQUES_SPEEDS 20
#define UP_DRIVE_MODES 20
#define UP_ORION_INFO 32
/* END AUTO UP DEFS */

#define CHECK_STALE(stale, curr, last, period) if(!stale && \
                    (curr - last) > period * STALE_THRESH) stale = 1

/* BEGIN AUTO CAN ENUMERATIONS */
typedef enum {
    CAR_STATE_IDLE,
    CAR_STATE_PRECHARGING,
    CAR_STATE_ENERGIZED,
    CAR_STATE_BUZZING,
    CAR_STATE_READY2DRIVE,
    CAR_STATE_ERROR,
    CAR_STATE_FATAL,
    CAR_STATE_RESET,
    CAR_STATE_RECOVER,
    CAR_STATE_CONSTANT_TORQUE,
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
    } main_hb_amk;
    struct {
        uint64_t AMK_Control_bReserve: 8;
        uint64_t AMK_Control_bInverterOn: 1;
        uint64_t AMK_Control_bDcOn: 1;
        uint64_t AMK_Control_bEnable: 1;
        uint64_t AMK_Control_bErrorReset: 1;
        uint64_t AMK_Control_bReserve2: 4;
        uint64_t AMK_TorqueSetpoint: 16;
        uint64_t AMK_PositiveTorqueLimit: 16;
        uint64_t AMK_NegativeTorqueLimit: 16;
    } INVA_SET;
    struct {
        uint64_t AMK_Control_bReserve: 8;
        uint64_t AMK_Control_bInverterOn: 1;
        uint64_t AMK_Control_bDcOn: 1;
        uint64_t AMK_Control_bEnable: 1;
        uint64_t AMK_Control_bErrorReset: 1;
        uint64_t AMK_Control_bReserve2: 4;
        uint64_t AMK_TorqueSetpoint: 16;
        uint64_t AMK_PositiveTorqueLimit: 16;
        uint64_t AMK_NegativeTorqueLimit: 16;
    } INVB_SET;
    struct {
        uint64_t car_state: 8;
        uint64_t precharge_state: 1;
    } main_hb;
    struct {
        uint64_t battery_in_temp: 8;
        uint64_t battery_out_temp: 8;
        uint64_t drivetrain_in_temp: 8;
        uint64_t drivetrain_out_temp: 8;
    } coolant_temps;
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
        uint64_t left_load_sensor: 32;
        uint64_t right_load_sensor: 32;
    } load_sensor_readings;
    struct {
        uint64_t left_shock: 16;
        uint64_t right_shock: 16;
    } shock_rear;
    struct {
        uint64_t sched_skips: 8;
        uint64_t foreground_use: 8;
        uint64_t background_use: 8;
        uint64_t sched_error: 8;
    } mcu_status;
    struct {
        uint64_t can1_tx_queue_overflow: 8;
        uint64_t can2_tx_queue_overflow: 8;
        uint64_t can1_tx_fail: 8;
        uint64_t can2_tx_fail: 8;
        uint64_t can_rx_queue_overflow: 8;
        uint64_t can1_rx_overrun: 8;
        uint64_t can2_rx_overrun: 8;
    } main_module_can_stats;
    struct {
        uint64_t noise_r: 16;
        uint64_t noise_l: 16;
    } num_mc_skips;
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
        uint64_t right_voltage: 16;
    } rear_motor_currents_volts;
    struct {
        uint64_t main: 1;
        uint64_t c_estop: 1;
        uint64_t inertia: 1;
        uint64_t BOTS: 1;
        uint64_t BSPD: 1;
        uint64_t BMS: 1;
        uint64_t IMD: 1;
        uint64_t r_estop: 1;
        uint64_t l_estop: 1;
        uint64_t HVD: 1;
        uint64_t emeter: 1;
        uint64_t hub: 1;
        uint64_t TSMS: 1;
        uint64_t pchg_out: 1;
    } sdc_status;
    struct {
        uint64_t left_mot_temp: 8;
        uint64_t right_mot_temp: 8;
        uint64_t left_inv_temp: 8;
        uint64_t right_inv_temp: 8;
        uint64_t left_igbt_temp: 8;
        uint64_t right_igbt_temp: 8;
    } rear_motor_temps;
    struct {
        uint64_t left_speed_sensor: 16;
        uint64_t right_speed_sensor: 16;
    } rear_wheel_speeds;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_main_module;
    struct {
        uint64_t daq_response: 64;
    } daq_response_MAIN_MODULE_VCAN;
    struct {
        uint64_t AMK_ActualSpeed: 16;
        uint64_t AMK_ActualTorque: 16;
        uint64_t AMK_DisplayOverloadInverter: 16;
        uint64_t AMK_DisplayOverloadMotor: 16;
    } INVA_CRIT;
    struct {
        uint64_t AMK_Status_bReserve: 8;
        uint64_t AMK_Status_bSystemReady: 1;
        uint64_t AMK_Status_bError: 1;
        uint64_t AMK_Status_bWarn: 1;
        uint64_t AMK_Status_bQuitDcOn: 1;
        uint64_t AMK_Status_bDcOn: 1;
        uint64_t AMK_Status_bQuitInverterOn: 1;
        uint64_t AMK_Status_bInverterOn: 1;
        uint64_t AMK_Status_bDerating: 1;
        uint64_t AMK_DCBusVoltage: 16;
    } INVA_INFO;
    struct {
        uint64_t AMK_MotorTemp: 16;
        uint64_t AMK_InverterTemp: 16;
        uint64_t AMK_IGBTTemp: 16;
    } INVA_TEMPS;
    struct {
        uint64_t AMK_DiagnosticNumber: 32;
        uint64_t AMK_ErrorInfo1: 32;
    } INVA_ERR_1;
    struct {
        uint64_t AMK_ErrorInfo2: 32;
        uint64_t AMK_ErrorInfo3: 32;
    } INVA_ERR_2;
    struct {
        uint64_t AMK_ActualSpeed: 16;
        uint64_t AMK_ActualTorque: 16;
        uint64_t AMK_DisplayOverloadInverter: 16;
        uint64_t AMK_DisplayOverloadMotor: 16;
    } INVB_CRIT;
    struct {
        uint64_t AMK_Status_bReserve: 8;
        uint64_t AMK_Status_bSystemReady: 1;
        uint64_t AMK_Status_bError: 1;
        uint64_t AMK_Status_bWarn: 1;
        uint64_t AMK_Status_bQuitDcOn: 1;
        uint64_t AMK_Status_bDcOn: 1;
        uint64_t AMK_Status_bQuitInverterOn: 1;
        uint64_t AMK_Status_bInverterOn: 1;
        uint64_t AMK_Status_bDerating: 1;
        uint64_t AMK_DCBusVoltage: 16;
    } INVB_INFO;
    struct {
        uint64_t AMK_MotorTemp: 16;
        uint64_t AMK_InverterTemp: 16;
        uint64_t AMK_IGBTTemp: 16;
    } INVB_TEMPS;
    struct {
        uint64_t AMK_DiagnosticNumber: 32;
        uint64_t AMK_ErrorInfo1: 32;
    } INVB_ERR_1;
    struct {
        uint64_t AMK_ErrorInfo2: 32;
        uint64_t AMK_ErrorInfo3: 32;
    } INVB_ERR_2;
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
        uint64_t pack_current: 16;
        uint64_t pack_voltage: 16;
    } orion_currents_volts;
    struct {
        uint64_t TO_VT_left: 16;
        uint64_t TO_VT_right: 16;
        uint64_t TO_PT_equal: 16;
        uint64_t VCU_mode: 8;
    } VCU_torques_speeds;
    struct {
        uint64_t VT_mode: 8;
        uint64_t WS_VS_equal: 16;
    } drive_modes;
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
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_pdu;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_dashboard;
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync_a_box;
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
    } daq_command_MAIN_MODULE_VCAN;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;
/* END AUTO MESSAGE STRUCTURE */

// contains most up to date received
// type for each variable matches that defined in JSON
/* BEGIN AUTO CAN DATA STRUCTURE */
typedef struct {
    struct {
        int16_t AMK_ActualSpeed;
        int16_t AMK_ActualTorque;
        uint16_t AMK_DisplayOverloadInverter;
        uint16_t AMK_DisplayOverloadMotor;
        uint8_t stale;
        uint32_t last_rx;
    } INVA_CRIT;
    struct {
        uint8_t AMK_Status_bReserve;
        uint8_t AMK_Status_bSystemReady;
        uint8_t AMK_Status_bError;
        uint8_t AMK_Status_bWarn;
        uint8_t AMK_Status_bQuitDcOn;
        uint8_t AMK_Status_bDcOn;
        uint8_t AMK_Status_bQuitInverterOn;
        uint8_t AMK_Status_bInverterOn;
        uint8_t AMK_Status_bDerating;
        uint16_t AMK_DCBusVoltage;
        uint8_t stale;
        uint32_t last_rx;
    } INVA_INFO;
    struct {
        int16_t AMK_MotorTemp;
        int16_t AMK_InverterTemp;
        int16_t AMK_IGBTTemp;
        uint8_t stale;
        uint32_t last_rx;
    } INVA_TEMPS;
    struct {
        uint8_t AMK_Control_bReserve;
        uint8_t AMK_Control_bInverterOn;
        uint8_t AMK_Control_bDcOn;
        uint8_t AMK_Control_bEnable;
        uint8_t AMK_Control_bErrorReset;
        uint8_t AMK_Control_bReserve2;
        int16_t AMK_TorqueSetpoint;
        int16_t AMK_PositiveTorqueLimit;
        int16_t AMK_NegativeTorqueLimit;
        uint8_t stale;
        uint32_t last_rx;
    } INVA_SET;
    struct {
        uint32_t AMK_DiagnosticNumber;
        uint32_t AMK_ErrorInfo1;
        uint8_t stale;
        uint32_t last_rx;
    } INVA_ERR_1;
    struct {
        uint32_t AMK_ErrorInfo2;
        uint32_t AMK_ErrorInfo3;
        uint8_t stale;
        uint32_t last_rx;
    } INVA_ERR_2;
    struct {
        int16_t AMK_ActualSpeed;
        int16_t AMK_ActualTorque;
        uint16_t AMK_DisplayOverloadInverter;
        uint16_t AMK_DisplayOverloadMotor;
        uint8_t stale;
        uint32_t last_rx;
    } INVB_CRIT;
    struct {
        uint8_t AMK_Status_bReserve;
        uint8_t AMK_Status_bSystemReady;
        uint8_t AMK_Status_bError;
        uint8_t AMK_Status_bWarn;
        uint8_t AMK_Status_bQuitDcOn;
        uint8_t AMK_Status_bDcOn;
        uint8_t AMK_Status_bQuitInverterOn;
        uint8_t AMK_Status_bInverterOn;
        uint8_t AMK_Status_bDerating;
        uint16_t AMK_DCBusVoltage;
        uint8_t stale;
        uint32_t last_rx;
    } INVB_INFO;
    struct {
        int16_t AMK_MotorTemp;
        int16_t AMK_InverterTemp;
        int16_t AMK_IGBTTemp;
        uint8_t stale;
        uint32_t last_rx;
    } INVB_TEMPS;
    struct {
        uint8_t AMK_Control_bReserve;
        uint8_t AMK_Control_bInverterOn;
        uint8_t AMK_Control_bDcOn;
        uint8_t AMK_Control_bEnable;
        uint8_t AMK_Control_bErrorReset;
        uint8_t AMK_Control_bReserve2;
        int16_t AMK_TorqueSetpoint;
        int16_t AMK_PositiveTorqueLimit;
        int16_t AMK_NegativeTorqueLimit;
        uint8_t stale;
        uint32_t last_rx;
    } INVB_SET;
    struct {
        uint32_t AMK_DiagnosticNumber;
        uint32_t AMK_ErrorInfo1;
        uint8_t stale;
        uint32_t last_rx;
    } INVB_ERR_1;
    struct {
        uint32_t AMK_ErrorInfo2;
        uint32_t AMK_ErrorInfo3;
        uint8_t stale;
        uint32_t last_rx;
    } INVB_ERR_2;
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
        int16_t max_temp;
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
        int16_t pack_current;
        uint16_t pack_voltage;
        uint8_t stale;
        uint32_t last_rx;
    } orion_currents_volts;
    struct {
        int16_t TO_VT_left;
        int16_t TO_VT_right;
        int16_t TO_PT_equal;
        int16_t WS_VS_equal;
        uint8_t stale;
        uint32_t last_rx;
    } VCU_torques_speeds;
    struct {
        int8_t VCU_mode;
        int8_t VT_mode;
        uint8_t stale;
        uint32_t last_rx;
    } drive_modes;
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
        uint16_t idx;
        uint8_t latched;
    } fault_sync_pdu;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_dashboard;
    struct {
        uint16_t idx;
        uint8_t latched;
    } fault_sync_a_box;
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
    } daq_command_MAIN_MODULE_VCAN;
} can_data_t;
/* END AUTO CAN DATA STRUCTURE */

extern can_data_t can_data;
extern volatile uint32_t last_can_rx_time_ms;

/* BEGIN AUTO EXTERN CALLBACK */
extern void daq_command_MAIN_MODULE_VCAN_CALLBACK(CanMsgTypeDef_t* msg_header_a);
extern void main_module_bl_cmd_CALLBACK(CanParsedData_t* msg_data_a);
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
void canRxUpdate(void);

/**
 * @brief Process any rx message callbacks from the CAN Rx IRQ
 *
 * @param rx rx data from message just recieved
 */
void canProcessRxIRQs(CanMsgTypeDef_t* rx);

#endif
