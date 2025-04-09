/**
 * @file faults.c
 * @author Aditya Anand (anand89@purdue.edu)
 * @brief Creating a library of faults to create an easy to debug atmosphere on the car
 * @version 0.1
 * @date 2022-05-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "faults.h"
#if defined(STM32F407xx)
#include "common/phal_F4_F7/can/can.h"
#elif defined(STM32F732xx)
#include "common/phal_F4_F7/can/can.h"
#else
#include "common/phal_L4/can/can.h"
#endif
#include "common/psched/psched.h"

//BEGIN AUTO FAULT INFO ARRAY DEFS
uint16_t faultLatchTime[TOTAL_NUM_FAULTS] = { PDU_MCU_TEMP_HIGH_LATCH_TIME, DASH_RAIL_LATCH_TIME, ABOX_RAIL_LATCH_TIME, MAIN_RAIL_LATCH_TIME, DAQ_RAIL_LATCH_TIME, V_CRIT_LATCH_TIME,
			V_NONCRIT_LATCH_TIME, FAN1_LATCH_TIME, BULLET_RAIL_LATCH_TIME, FAN2_LATCH_TIME, LV_CRITICAL_LOW_LATCH_TIME, LV_GETTING_LOW_LATCH_TIME, LV_BATT_FIFTY_LATCH_TIME,
			REMAP_UNRELIABLE_LATCH_TIME, RTD_EXIT_LATCH_TIME, LEFT_MC_CONN_LATCH_TIME, RIGHT_MC_CONN_LATCH_TIME, MCU_TEMP_HIGH_LATCH_TIME, DT_L_TEMP_OT_LATCH_TIME, DT_R_TEMP_OT_LATCH_TIME,
			DT_L_TEMP_HIGH_LATCH_TIME, DT_R_TEMP_HIGH_LATCH_TIME, BAT_COOL_LOOP_HIGH_LATCH_TIME, DT_COOL_LOOP_HIGH_LATCH_TIME, DT_R_THERM_DISC_LATCH_TIME, DT_L_THERM_DISC_LATCH_TIME, BATT_CL_DISC_LATCH_TIME,
			DT_CL_DISC_LATCH_TIME, BSPD_LATCHED_LATCH_TIME, BOTS_FAIL_LATCH_TIME, INERTIA_FAIL_LATCH_TIME, COCKPIT_ESTOP_LATCH_TIME, RIGHT_ESTOP_LATCH_TIME, LEFT_ESTOP_LATCH_TIME,
			HVD_DISC_LATCH_TIME, HUB_DISC_LATCH_TIME, TSMS_DISC_LATCH_TIME, PRECHARGE_TIME_FAULT_LATCH_TIME, MOTOR_R_HEAT_LATCH_TIME, MOTOR_L_HEAT_LATCH_TIME, MOTOR_R_OT_LATCH_TIME,
			MOTOR_L_OT_LATCH_TIME, APPS_WIRING_T1_LATCH_TIME, APPS_WIRING_T2_LATCH_TIME, BSE_LATCH_TIME, BSPD_LATCH_TIME, IMPLAUS_DETECTED_LATCH_TIME, APPS_BRAKE_LATCH_TIME,
			DISCHARGE_LIMIT_ENFORCE_LATCH_TIME, CHARGER_SAFETY_RELAY_LATCH_TIME, INTERNAL_HARDWARE_LATCH_TIME, HEATSINK_THERMISTOR_LATCH_TIME, SOFTWARE_LATCH_TIME, MAX_CELLV_HIGH_LATCH_TIME, MIN_CELLV_LOW_LATCH_TIME,
			PACK_OVERHEAT_ORION_LATCH_TIME, INTERNAL_COMMS_LATCH_TIME, CELL_BALANCING_FOFF_LATCH_TIME, WEAK_CELL_LATCH_TIME, LOW_CELLV_LATCH_TIME, OPEN_WIRE_LATCH_TIME, CURRENT_SENSOR_LATCH_TIME,
			MAX_CELLV_O5V_LATCH_TIME, CELL_ASIC_LATCH_TIME, WEAK_PACK_LATCH_TIME, FAN_MONITOR_LATCH_TIME, THERMISTOR_LATCH_TIME, EXTERNAL_COMMS_LATCH_TIME, REDUNDANT_PSU_LATCH_TIME,
			HV_ISOLATION_LATCH_TIME, INPUT_PSU_LATCH_TIME, CHARGE_LIMIT_ENFORCE_LATCH_TIME, PACK_TEMP_LATCH_TIME, PACK_TEMP_EXCEEDED_LATCH_TIME, MIN_PACK_TEMP_LATCH_TIME, IMD_LATCH_TIME,
			TMU_POWER_LOST_LATCH_TIME, ET_ENABLED_LATCH_TIME, PT_ENABLED_LATCH_TIME, VS_ENABLED_LATCH_TIME, VT_ENABLED_LATCH_TIME, NO_GPS_FIX_LATCH_TIME, YES_GPS_FIX_LATCH_TIME,
			TEST_FAULT_1_LATCH_TIME, TEST_FAULT_2_LATCH_TIME, TEST_FAULT_3_LATCH_TIME, TEST_FAULT_4_LATCH_TIME,};
uint16_t faultULatchTime[TOTAL_NUM_FAULTS] = { PDU_MCU_TEMP_HIGH_UNLATCH_TIME, DASH_RAIL_UNLATCH_TIME, ABOX_RAIL_UNLATCH_TIME, MAIN_RAIL_UNLATCH_TIME, DAQ_RAIL_UNLATCH_TIME, V_CRIT_UNLATCH_TIME,
			V_NONCRIT_UNLATCH_TIME, FAN1_UNLATCH_TIME, BULLET_RAIL_UNLATCH_TIME, FAN2_UNLATCH_TIME, LV_CRITICAL_LOW_UNLATCH_TIME, LV_GETTING_LOW_UNLATCH_TIME, LV_BATT_FIFTY_UNLATCH_TIME,
			REMAP_UNRELIABLE_UNLATCH_TIME, RTD_EXIT_UNLATCH_TIME, LEFT_MC_CONN_UNLATCH_TIME, RIGHT_MC_CONN_UNLATCH_TIME, MCU_TEMP_HIGH_UNLATCH_TIME, DT_L_TEMP_OT_UNLATCH_TIME, DT_R_TEMP_OT_UNLATCH_TIME,
			DT_L_TEMP_HIGH_UNLATCH_TIME, DT_R_TEMP_HIGH_UNLATCH_TIME, BAT_COOL_LOOP_HIGH_UNLATCH_TIME, DT_COOL_LOOP_HIGH_UNLATCH_TIME, DT_R_THERM_DISC_UNLATCH_TIME, DT_L_THERM_DISC_UNLATCH_TIME, BATT_CL_DISC_UNLATCH_TIME,
			DT_CL_DISC_UNLATCH_TIME, BSPD_LATCHED_UNLATCH_TIME, BOTS_FAIL_UNLATCH_TIME, INERTIA_FAIL_UNLATCH_TIME, COCKPIT_ESTOP_UNLATCH_TIME, RIGHT_ESTOP_UNLATCH_TIME, LEFT_ESTOP_UNLATCH_TIME,
			HVD_DISC_UNLATCH_TIME, HUB_DISC_UNLATCH_TIME, TSMS_DISC_UNLATCH_TIME, PRECHARGE_TIME_FAULT_UNLATCH_TIME, MOTOR_R_HEAT_UNLATCH_TIME, MOTOR_L_HEAT_UNLATCH_TIME, MOTOR_R_OT_UNLATCH_TIME,
			MOTOR_L_OT_UNLATCH_TIME, APPS_WIRING_T1_UNLATCH_TIME, APPS_WIRING_T2_UNLATCH_TIME, BSE_UNLATCH_TIME, BSPD_UNLATCH_TIME, IMPLAUS_DETECTED_UNLATCH_TIME, APPS_BRAKE_UNLATCH_TIME,
			DISCHARGE_LIMIT_ENFORCE_UNLATCH_TIME, CHARGER_SAFETY_RELAY_UNLATCH_TIME, INTERNAL_HARDWARE_UNLATCH_TIME, HEATSINK_THERMISTOR_UNLATCH_TIME, SOFTWARE_UNLATCH_TIME, MAX_CELLV_HIGH_UNLATCH_TIME, MIN_CELLV_LOW_UNLATCH_TIME,
			PACK_OVERHEAT_ORION_UNLATCH_TIME, INTERNAL_COMMS_UNLATCH_TIME, CELL_BALANCING_FOFF_UNLATCH_TIME, WEAK_CELL_UNLATCH_TIME, LOW_CELLV_UNLATCH_TIME, OPEN_WIRE_UNLATCH_TIME, CURRENT_SENSOR_UNLATCH_TIME,
			MAX_CELLV_O5V_UNLATCH_TIME, CELL_ASIC_UNLATCH_TIME, WEAK_PACK_UNLATCH_TIME, FAN_MONITOR_UNLATCH_TIME, THERMISTOR_UNLATCH_TIME, EXTERNAL_COMMS_UNLATCH_TIME, REDUNDANT_PSU_UNLATCH_TIME,
			HV_ISOLATION_UNLATCH_TIME, INPUT_PSU_UNLATCH_TIME, CHARGE_LIMIT_ENFORCE_UNLATCH_TIME, PACK_TEMP_UNLATCH_TIME, PACK_TEMP_EXCEEDED_UNLATCH_TIME, MIN_PACK_TEMP_UNLATCH_TIME, IMD_UNLATCH_TIME,
			TMU_POWER_LOST_UNLATCH_TIME, ET_ENABLED_UNLATCH_TIME, PT_ENABLED_UNLATCH_TIME, VS_ENABLED_UNLATCH_TIME, VT_ENABLED_UNLATCH_TIME, NO_GPS_FIX_UNLATCH_TIME, YES_GPS_FIX_UNLATCH_TIME,
			TEST_FAULT_1_UNLATCH_TIME, TEST_FAULT_2_UNLATCH_TIME, TEST_FAULT_3_UNLATCH_TIME, TEST_FAULT_4_UNLATCH_TIME,};
//Global arrays with all faults
fault_status_t statusArray[TOTAL_NUM_FAULTS] = {
	(fault_status_t){false, ID_PDU_MCU_TEMP_HIGH_FAULT},
	(fault_status_t){false, ID_DASH_RAIL_FAULT},
	(fault_status_t){false, ID_ABOX_RAIL_FAULT},
	(fault_status_t){false, ID_MAIN_RAIL_FAULT},
	(fault_status_t){false, ID_DAQ_RAIL_FAULT},
	(fault_status_t){false, ID_V_CRIT_FAULT},
	(fault_status_t){false, ID_V_NONCRIT_FAULT},
	(fault_status_t){false, ID_FAN1_FAULT},
	(fault_status_t){false, ID_BULLET_RAIL_FAULT},
	(fault_status_t){false, ID_FAN2_FAULT},
	(fault_status_t){false, ID_LV_CRITICAL_LOW_FAULT},
	(fault_status_t){false, ID_LV_GETTING_LOW_FAULT},
	(fault_status_t){false, ID_LV_BATT_FIFTY_FAULT},
	(fault_status_t){false, ID_REMAP_UNRELIABLE_FAULT},
	(fault_status_t){false, ID_RTD_EXIT_FAULT},
	(fault_status_t){false, ID_LEFT_MC_CONN_FAULT},
	(fault_status_t){false, ID_RIGHT_MC_CONN_FAULT},
	(fault_status_t){false, ID_MCU_TEMP_HIGH_FAULT},
	(fault_status_t){false, ID_DT_L_TEMP_OT_FAULT},
	(fault_status_t){false, ID_DT_R_TEMP_OT_FAULT},
	(fault_status_t){false, ID_DT_L_TEMP_HIGH_FAULT},
	(fault_status_t){false, ID_DT_R_TEMP_HIGH_FAULT},
	(fault_status_t){false, ID_BAT_COOL_LOOP_HIGH_FAULT},
	(fault_status_t){false, ID_DT_COOL_LOOP_HIGH_FAULT},
	(fault_status_t){false, ID_DT_R_THERM_DISC_FAULT},
	(fault_status_t){false, ID_DT_L_THERM_DISC_FAULT},
	(fault_status_t){false, ID_BATT_CL_DISC_FAULT},
	(fault_status_t){false, ID_DT_CL_DISC_FAULT},
	(fault_status_t){false, ID_BSPD_LATCHED_FAULT},
	(fault_status_t){false, ID_BOTS_FAIL_FAULT},
	(fault_status_t){false, ID_INERTIA_FAIL_FAULT},
	(fault_status_t){false, ID_COCKPIT_ESTOP_FAULT},
	(fault_status_t){false, ID_RIGHT_ESTOP_FAULT},
	(fault_status_t){false, ID_LEFT_ESTOP_FAULT},
	(fault_status_t){false, ID_HVD_DISC_FAULT},
	(fault_status_t){false, ID_HUB_DISC_FAULT},
	(fault_status_t){false, ID_TSMS_DISC_FAULT},
	(fault_status_t){false, ID_PRECHARGE_TIME_FAULT_FAULT},
	(fault_status_t){false, ID_MOTOR_R_HEAT_FAULT},
	(fault_status_t){false, ID_MOTOR_L_HEAT_FAULT},
	(fault_status_t){false, ID_MOTOR_R_OT_FAULT},
	(fault_status_t){false, ID_MOTOR_L_OT_FAULT},
	(fault_status_t){false, ID_APPS_WIRING_T1_FAULT},
	(fault_status_t){false, ID_APPS_WIRING_T2_FAULT},
	(fault_status_t){false, ID_BSE_FAULT},
	(fault_status_t){false, ID_BSPD_FAULT},
	(fault_status_t){false, ID_IMPLAUS_DETECTED_FAULT},
	(fault_status_t){false, ID_APPS_BRAKE_FAULT},
	(fault_status_t){false, ID_DISCHARGE_LIMIT_ENFORCE_FAULT},
	(fault_status_t){false, ID_CHARGER_SAFETY_RELAY_FAULT},
	(fault_status_t){false, ID_INTERNAL_HARDWARE_FAULT},
	(fault_status_t){false, ID_HEATSINK_THERMISTOR_FAULT},
	(fault_status_t){false, ID_SOFTWARE_FAULT},
	(fault_status_t){false, ID_MAX_CELLV_HIGH_FAULT},
	(fault_status_t){false, ID_MIN_CELLV_LOW_FAULT},
	(fault_status_t){false, ID_PACK_OVERHEAT_ORION_FAULT},
	(fault_status_t){false, ID_INTERNAL_COMMS_FAULT},
	(fault_status_t){false, ID_CELL_BALANCING_FOFF_FAULT},
	(fault_status_t){false, ID_WEAK_CELL_FAULT},
	(fault_status_t){false, ID_LOW_CELLV_FAULT},
	(fault_status_t){false, ID_OPEN_WIRE_FAULT},
	(fault_status_t){false, ID_CURRENT_SENSOR_FAULT},
	(fault_status_t){false, ID_MAX_CELLV_O5V_FAULT},
	(fault_status_t){false, ID_CELL_ASIC_FAULT},
	(fault_status_t){false, ID_WEAK_PACK_FAULT},
	(fault_status_t){false, ID_FAN_MONITOR_FAULT},
	(fault_status_t){false, ID_THERMISTOR_FAULT},
	(fault_status_t){false, ID_EXTERNAL_COMMS_FAULT},
	(fault_status_t){false, ID_REDUNDANT_PSU_FAULT},
	(fault_status_t){false, ID_HV_ISOLATION_FAULT},
	(fault_status_t){false, ID_INPUT_PSU_FAULT},
	(fault_status_t){false, ID_CHARGE_LIMIT_ENFORCE_FAULT},
	(fault_status_t){false, ID_PACK_TEMP_FAULT},
	(fault_status_t){false, ID_PACK_TEMP_EXCEEDED_FAULT},
	(fault_status_t){false, ID_MIN_PACK_TEMP_FAULT},
	(fault_status_t){false, ID_IMD_FAULT},
	(fault_status_t){false, ID_TMU_POWER_LOST_FAULT},
	(fault_status_t){false, ID_ET_ENABLED_FAULT},
	(fault_status_t){false, ID_PT_ENABLED_FAULT},
	(fault_status_t){false, ID_VS_ENABLED_FAULT},
	(fault_status_t){false, ID_VT_ENABLED_FAULT},
	(fault_status_t){false, ID_NO_GPS_FIX_FAULT},
	(fault_status_t){false, ID_YES_GPS_FIX_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_1_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_2_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_3_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_4_FAULT},
};
fault_attributes_t faultArray[TOTAL_NUM_FAULTS] = {
	(fault_attributes_t){false, false, PDU_MCU_TEMP_HIGH_PRIORITY, 0, 0, PDU_MCU_TEMP_HIGH_MAX, PDU_MCU_TEMP_HIGH_MIN, &statusArray[0], 0, PDU_MCU_TEMP_HIGH_MSG}, 
	(fault_attributes_t){false, false, DASH_RAIL_PRIORITY, 0, 0, DASH_RAIL_MAX, DASH_RAIL_MIN, &statusArray[1], 0, DASH_RAIL_MSG}, 
	(fault_attributes_t){false, false, ABOX_RAIL_PRIORITY, 0, 0, ABOX_RAIL_MAX, ABOX_RAIL_MIN, &statusArray[2], 0, ABOX_RAIL_MSG}, 
	(fault_attributes_t){false, false, MAIN_RAIL_PRIORITY, 0, 0, MAIN_RAIL_MAX, MAIN_RAIL_MIN, &statusArray[3], 0, MAIN_RAIL_MSG}, 
	(fault_attributes_t){false, false, DAQ_RAIL_PRIORITY, 0, 0, DAQ_RAIL_MAX, DAQ_RAIL_MIN, &statusArray[4], 0, DAQ_RAIL_MSG}, 
	(fault_attributes_t){false, false, V_CRIT_PRIORITY, 0, 0, V_CRIT_MAX, V_CRIT_MIN, &statusArray[5], 0, V_CRIT_MSG}, 
	(fault_attributes_t){false, false, V_NONCRIT_PRIORITY, 0, 0, V_NONCRIT_MAX, V_NONCRIT_MIN, &statusArray[6], 0, V_NONCRIT_MSG}, 
	(fault_attributes_t){false, false, FAN1_PRIORITY, 0, 0, FAN1_MAX, FAN1_MIN, &statusArray[7], 0, FAN1_MSG}, 
	(fault_attributes_t){false, false, BULLET_RAIL_PRIORITY, 0, 0, BULLET_RAIL_MAX, BULLET_RAIL_MIN, &statusArray[8], 0, BULLET_RAIL_MSG}, 
	(fault_attributes_t){false, false, FAN2_PRIORITY, 0, 0, FAN2_MAX, FAN2_MIN, &statusArray[9], 0, FAN2_MSG}, 
	(fault_attributes_t){false, false, LV_CRITICAL_LOW_PRIORITY, 0, 0, LV_CRITICAL_LOW_MAX, LV_CRITICAL_LOW_MIN, &statusArray[10], 0, LV_CRITICAL_LOW_MSG}, 
	(fault_attributes_t){false, false, LV_GETTING_LOW_PRIORITY, 0, 0, LV_GETTING_LOW_MAX, LV_GETTING_LOW_MIN, &statusArray[11], 0, LV_GETTING_LOW_MSG}, 
	(fault_attributes_t){false, false, LV_BATT_FIFTY_PRIORITY, 0, 0, LV_BATT_FIFTY_MAX, LV_BATT_FIFTY_MIN, &statusArray[12], 0, LV_BATT_FIFTY_MSG}, 
	(fault_attributes_t){false, false, REMAP_UNRELIABLE_PRIORITY, 0, 0, REMAP_UNRELIABLE_MAX, REMAP_UNRELIABLE_MIN, &statusArray[13], 0, REMAP_UNRELIABLE_MSG}, 
	(fault_attributes_t){false, false, RTD_EXIT_PRIORITY, 0, 0, RTD_EXIT_MAX, RTD_EXIT_MIN, &statusArray[14], 0, RTD_EXIT_MSG}, 
	(fault_attributes_t){false, false, LEFT_MC_CONN_PRIORITY, 0, 0, LEFT_MC_CONN_MAX, LEFT_MC_CONN_MIN, &statusArray[15], 0, LEFT_MC_CONN_MSG}, 
	(fault_attributes_t){false, false, RIGHT_MC_CONN_PRIORITY, 0, 0, RIGHT_MC_CONN_MAX, RIGHT_MC_CONN_MIN, &statusArray[16], 0, RIGHT_MC_CONN_MSG}, 
	(fault_attributes_t){false, false, MCU_TEMP_HIGH_PRIORITY, 0, 0, MCU_TEMP_HIGH_MAX, MCU_TEMP_HIGH_MIN, &statusArray[17], 0, MCU_TEMP_HIGH_MSG}, 
	(fault_attributes_t){false, false, DT_L_TEMP_OT_PRIORITY, 0, 0, DT_L_TEMP_OT_MAX, DT_L_TEMP_OT_MIN, &statusArray[18], 0, DT_L_TEMP_OT_MSG}, 
	(fault_attributes_t){false, false, DT_R_TEMP_OT_PRIORITY, 0, 0, DT_R_TEMP_OT_MAX, DT_R_TEMP_OT_MIN, &statusArray[19], 0, DT_R_TEMP_OT_MSG}, 
	(fault_attributes_t){false, false, DT_L_TEMP_HIGH_PRIORITY, 0, 0, DT_L_TEMP_HIGH_MAX, DT_L_TEMP_HIGH_MIN, &statusArray[20], 0, DT_L_TEMP_HIGH_MSG}, 
	(fault_attributes_t){false, false, DT_R_TEMP_HIGH_PRIORITY, 0, 0, DT_R_TEMP_HIGH_MAX, DT_R_TEMP_HIGH_MIN, &statusArray[21], 0, DT_R_TEMP_HIGH_MSG}, 
	(fault_attributes_t){false, false, BAT_COOL_LOOP_HIGH_PRIORITY, 0, 0, BAT_COOL_LOOP_HIGH_MAX, BAT_COOL_LOOP_HIGH_MIN, &statusArray[22], 0, BAT_COOL_LOOP_HIGH_MSG}, 
	(fault_attributes_t){false, false, DT_COOL_LOOP_HIGH_PRIORITY, 0, 0, DT_COOL_LOOP_HIGH_MAX, DT_COOL_LOOP_HIGH_MIN, &statusArray[23], 0, DT_COOL_LOOP_HIGH_MSG}, 
	(fault_attributes_t){false, false, DT_R_THERM_DISC_PRIORITY, 0, 0, DT_R_THERM_DISC_MAX, DT_R_THERM_DISC_MIN, &statusArray[24], 0, DT_R_THERM_DISC_MSG}, 
	(fault_attributes_t){false, false, DT_L_THERM_DISC_PRIORITY, 0, 0, DT_L_THERM_DISC_MAX, DT_L_THERM_DISC_MIN, &statusArray[25], 0, DT_L_THERM_DISC_MSG}, 
	(fault_attributes_t){false, false, BATT_CL_DISC_PRIORITY, 0, 0, BATT_CL_DISC_MAX, BATT_CL_DISC_MIN, &statusArray[26], 0, BATT_CL_DISC_MSG}, 
	(fault_attributes_t){false, false, DT_CL_DISC_PRIORITY, 0, 0, DT_CL_DISC_MAX, DT_CL_DISC_MIN, &statusArray[27], 0, DT_CL_DISC_MSG}, 
	(fault_attributes_t){false, false, BSPD_LATCHED_PRIORITY, 0, 0, BSPD_LATCHED_MAX, BSPD_LATCHED_MIN, &statusArray[28], 0, BSPD_LATCHED_MSG}, 
	(fault_attributes_t){false, false, BOTS_FAIL_PRIORITY, 0, 0, BOTS_FAIL_MAX, BOTS_FAIL_MIN, &statusArray[29], 0, BOTS_FAIL_MSG}, 
	(fault_attributes_t){false, false, INERTIA_FAIL_PRIORITY, 0, 0, INERTIA_FAIL_MAX, INERTIA_FAIL_MIN, &statusArray[30], 0, INERTIA_FAIL_MSG}, 
	(fault_attributes_t){false, false, COCKPIT_ESTOP_PRIORITY, 0, 0, COCKPIT_ESTOP_MAX, COCKPIT_ESTOP_MIN, &statusArray[31], 0, COCKPIT_ESTOP_MSG}, 
	(fault_attributes_t){false, false, RIGHT_ESTOP_PRIORITY, 0, 0, RIGHT_ESTOP_MAX, RIGHT_ESTOP_MIN, &statusArray[32], 0, RIGHT_ESTOP_MSG}, 
	(fault_attributes_t){false, false, LEFT_ESTOP_PRIORITY, 0, 0, LEFT_ESTOP_MAX, LEFT_ESTOP_MIN, &statusArray[33], 0, LEFT_ESTOP_MSG}, 
	(fault_attributes_t){false, false, HVD_DISC_PRIORITY, 0, 0, HVD_DISC_MAX, HVD_DISC_MIN, &statusArray[34], 0, HVD_DISC_MSG}, 
	(fault_attributes_t){false, false, HUB_DISC_PRIORITY, 0, 0, HUB_DISC_MAX, HUB_DISC_MIN, &statusArray[35], 0, HUB_DISC_MSG}, 
	(fault_attributes_t){false, false, TSMS_DISC_PRIORITY, 0, 0, TSMS_DISC_MAX, TSMS_DISC_MIN, &statusArray[36], 0, TSMS_DISC_MSG}, 
	(fault_attributes_t){false, false, PRECHARGE_TIME_FAULT_PRIORITY, 0, 0, PRECHARGE_TIME_FAULT_MAX, PRECHARGE_TIME_FAULT_MIN, &statusArray[37], 0, PRECHARGE_TIME_FAULT_MSG}, 
	(fault_attributes_t){false, false, MOTOR_R_HEAT_PRIORITY, 0, 0, MOTOR_R_HEAT_MAX, MOTOR_R_HEAT_MIN, &statusArray[38], 0, MOTOR_R_HEAT_MSG}, 
	(fault_attributes_t){false, false, MOTOR_L_HEAT_PRIORITY, 0, 0, MOTOR_L_HEAT_MAX, MOTOR_L_HEAT_MIN, &statusArray[39], 0, MOTOR_L_HEAT_MSG}, 
	(fault_attributes_t){false, false, MOTOR_R_OT_PRIORITY, 0, 0, MOTOR_R_OT_MAX, MOTOR_R_OT_MIN, &statusArray[40], 0, MOTOR_R_OT_MSG}, 
	(fault_attributes_t){false, false, MOTOR_L_OT_PRIORITY, 0, 0, MOTOR_L_OT_MAX, MOTOR_L_OT_MIN, &statusArray[41], 0, MOTOR_L_OT_MSG}, 
	(fault_attributes_t){false, false, APPS_WIRING_T1_PRIORITY, 0, 0, APPS_WIRING_T1_MAX, APPS_WIRING_T1_MIN, &statusArray[42], 0, APPS_WIRING_T1_MSG}, 
	(fault_attributes_t){false, false, APPS_WIRING_T2_PRIORITY, 0, 0, APPS_WIRING_T2_MAX, APPS_WIRING_T2_MIN, &statusArray[43], 0, APPS_WIRING_T2_MSG}, 
	(fault_attributes_t){false, false, BSE_PRIORITY, 0, 0, BSE_MAX, BSE_MIN, &statusArray[44], 0, BSE_MSG}, 
	(fault_attributes_t){false, false, BSPD_PRIORITY, 0, 0, BSPD_MAX, BSPD_MIN, &statusArray[45], 0, BSPD_MSG}, 
	(fault_attributes_t){false, false, IMPLAUS_DETECTED_PRIORITY, 0, 0, IMPLAUS_DETECTED_MAX, IMPLAUS_DETECTED_MIN, &statusArray[46], 0, IMPLAUS_DETECTED_MSG}, 
	(fault_attributes_t){false, false, APPS_BRAKE_PRIORITY, 0, 0, APPS_BRAKE_MAX, APPS_BRAKE_MIN, &statusArray[47], 0, APPS_BRAKE_MSG}, 
	(fault_attributes_t){false, false, DISCHARGE_LIMIT_ENFORCE_PRIORITY, 0, 0, DISCHARGE_LIMIT_ENFORCE_MAX, DISCHARGE_LIMIT_ENFORCE_MIN, &statusArray[48], 0, DISCHARGE_LIMIT_ENFORCE_MSG}, 
	(fault_attributes_t){false, false, CHARGER_SAFETY_RELAY_PRIORITY, 0, 0, CHARGER_SAFETY_RELAY_MAX, CHARGER_SAFETY_RELAY_MIN, &statusArray[49], 0, CHARGER_SAFETY_RELAY_MSG}, 
	(fault_attributes_t){false, false, INTERNAL_HARDWARE_PRIORITY, 0, 0, INTERNAL_HARDWARE_MAX, INTERNAL_HARDWARE_MIN, &statusArray[50], 0, INTERNAL_HARDWARE_MSG}, 
	(fault_attributes_t){false, false, HEATSINK_THERMISTOR_PRIORITY, 0, 0, HEATSINK_THERMISTOR_MAX, HEATSINK_THERMISTOR_MIN, &statusArray[51], 0, HEATSINK_THERMISTOR_MSG}, 
	(fault_attributes_t){false, false, SOFTWARE_PRIORITY, 0, 0, SOFTWARE_MAX, SOFTWARE_MIN, &statusArray[52], 0, SOFTWARE_MSG}, 
	(fault_attributes_t){false, false, MAX_CELLV_HIGH_PRIORITY, 0, 0, MAX_CELLV_HIGH_MAX, MAX_CELLV_HIGH_MIN, &statusArray[53], 0, MAX_CELLV_HIGH_MSG}, 
	(fault_attributes_t){false, false, MIN_CELLV_LOW_PRIORITY, 0, 0, MIN_CELLV_LOW_MAX, MIN_CELLV_LOW_MIN, &statusArray[54], 0, MIN_CELLV_LOW_MSG}, 
	(fault_attributes_t){false, false, PACK_OVERHEAT_ORION_PRIORITY, 0, 0, PACK_OVERHEAT_ORION_MAX, PACK_OVERHEAT_ORION_MIN, &statusArray[55], 0, PACK_OVERHEAT_ORION_MSG}, 
	(fault_attributes_t){false, false, INTERNAL_COMMS_PRIORITY, 0, 0, INTERNAL_COMMS_MAX, INTERNAL_COMMS_MIN, &statusArray[56], 0, INTERNAL_COMMS_MSG}, 
	(fault_attributes_t){false, false, CELL_BALANCING_FOFF_PRIORITY, 0, 0, CELL_BALANCING_FOFF_MAX, CELL_BALANCING_FOFF_MIN, &statusArray[57], 0, CELL_BALANCING_FOFF_MSG}, 
	(fault_attributes_t){false, false, WEAK_CELL_PRIORITY, 0, 0, WEAK_CELL_MAX, WEAK_CELL_MIN, &statusArray[58], 0, WEAK_CELL_MSG}, 
	(fault_attributes_t){false, false, LOW_CELLV_PRIORITY, 0, 0, LOW_CELLV_MAX, LOW_CELLV_MIN, &statusArray[59], 0, LOW_CELLV_MSG}, 
	(fault_attributes_t){false, false, OPEN_WIRE_PRIORITY, 0, 0, OPEN_WIRE_MAX, OPEN_WIRE_MIN, &statusArray[60], 0, OPEN_WIRE_MSG}, 
	(fault_attributes_t){false, false, CURRENT_SENSOR_PRIORITY, 0, 0, CURRENT_SENSOR_MAX, CURRENT_SENSOR_MIN, &statusArray[61], 0, CURRENT_SENSOR_MSG}, 
	(fault_attributes_t){false, false, MAX_CELLV_O5V_PRIORITY, 0, 0, MAX_CELLV_O5V_MAX, MAX_CELLV_O5V_MIN, &statusArray[62], 0, MAX_CELLV_O5V_MSG}, 
	(fault_attributes_t){false, false, CELL_ASIC_PRIORITY, 0, 0, CELL_ASIC_MAX, CELL_ASIC_MIN, &statusArray[63], 0, CELL_ASIC_MSG}, 
	(fault_attributes_t){false, false, WEAK_PACK_PRIORITY, 0, 0, WEAK_PACK_MAX, WEAK_PACK_MIN, &statusArray[64], 0, WEAK_PACK_MSG}, 
	(fault_attributes_t){false, false, FAN_MONITOR_PRIORITY, 0, 0, FAN_MONITOR_MAX, FAN_MONITOR_MIN, &statusArray[65], 0, FAN_MONITOR_MSG}, 
	(fault_attributes_t){false, false, THERMISTOR_PRIORITY, 0, 0, THERMISTOR_MAX, THERMISTOR_MIN, &statusArray[66], 0, THERMISTOR_MSG}, 
	(fault_attributes_t){false, false, EXTERNAL_COMMS_PRIORITY, 0, 0, EXTERNAL_COMMS_MAX, EXTERNAL_COMMS_MIN, &statusArray[67], 0, EXTERNAL_COMMS_MSG}, 
	(fault_attributes_t){false, false, REDUNDANT_PSU_PRIORITY, 0, 0, REDUNDANT_PSU_MAX, REDUNDANT_PSU_MIN, &statusArray[68], 0, REDUNDANT_PSU_MSG}, 
	(fault_attributes_t){false, false, HV_ISOLATION_PRIORITY, 0, 0, HV_ISOLATION_MAX, HV_ISOLATION_MIN, &statusArray[69], 0, HV_ISOLATION_MSG}, 
	(fault_attributes_t){false, false, INPUT_PSU_PRIORITY, 0, 0, INPUT_PSU_MAX, INPUT_PSU_MIN, &statusArray[70], 0, INPUT_PSU_MSG}, 
	(fault_attributes_t){false, false, CHARGE_LIMIT_ENFORCE_PRIORITY, 0, 0, CHARGE_LIMIT_ENFORCE_MAX, CHARGE_LIMIT_ENFORCE_MIN, &statusArray[71], 0, CHARGE_LIMIT_ENFORCE_MSG}, 
	(fault_attributes_t){false, false, PACK_TEMP_PRIORITY, 0, 0, PACK_TEMP_MAX, PACK_TEMP_MIN, &statusArray[72], 0, PACK_TEMP_MSG}, 
	(fault_attributes_t){false, false, PACK_TEMP_EXCEEDED_PRIORITY, 0, 0, PACK_TEMP_EXCEEDED_MAX, PACK_TEMP_EXCEEDED_MIN, &statusArray[73], 0, PACK_TEMP_EXCEEDED_MSG}, 
	(fault_attributes_t){false, false, MIN_PACK_TEMP_PRIORITY, 0, 0, MIN_PACK_TEMP_MAX, MIN_PACK_TEMP_MIN, &statusArray[74], 0, MIN_PACK_TEMP_MSG}, 
	(fault_attributes_t){false, false, IMD_PRIORITY, 0, 0, IMD_MAX, IMD_MIN, &statusArray[75], 0, IMD_MSG}, 
	(fault_attributes_t){false, false, TMU_POWER_LOST_PRIORITY, 0, 0, TMU_POWER_LOST_MAX, TMU_POWER_LOST_MIN, &statusArray[76], 0, TMU_POWER_LOST_MSG}, 
	(fault_attributes_t){false, false, ET_ENABLED_PRIORITY, 0, 0, ET_ENABLED_MAX, ET_ENABLED_MIN, &statusArray[77], 0, ET_ENABLED_MSG}, 
	(fault_attributes_t){false, false, PT_ENABLED_PRIORITY, 0, 0, PT_ENABLED_MAX, PT_ENABLED_MIN, &statusArray[78], 0, PT_ENABLED_MSG}, 
	(fault_attributes_t){false, false, VS_ENABLED_PRIORITY, 0, 0, VS_ENABLED_MAX, VS_ENABLED_MIN, &statusArray[79], 0, VS_ENABLED_MSG}, 
	(fault_attributes_t){false, false, VT_ENABLED_PRIORITY, 0, 0, VT_ENABLED_MAX, VT_ENABLED_MIN, &statusArray[80], 0, VT_ENABLED_MSG}, 
	(fault_attributes_t){false, false, NO_GPS_FIX_PRIORITY, 0, 0, NO_GPS_FIX_MAX, NO_GPS_FIX_MIN, &statusArray[81], 0, NO_GPS_FIX_MSG}, 
	(fault_attributes_t){false, false, YES_GPS_FIX_PRIORITY, 0, 0, YES_GPS_FIX_MAX, YES_GPS_FIX_MIN, &statusArray[82], 0, YES_GPS_FIX_MSG}, 
	(fault_attributes_t){false, false, TEST_FAULT_1_PRIORITY, 0, 0, TEST_FAULT_1_MAX, TEST_FAULT_1_MIN, &statusArray[83], 0, TEST_FAULT_1_MSG}, 
	(fault_attributes_t){false, false, TEST_FAULT_2_PRIORITY, 0, 0, TEST_FAULT_2_MAX, TEST_FAULT_2_MIN, &statusArray[84], 0, TEST_FAULT_2_MSG}, 
	(fault_attributes_t){false, false, TEST_FAULT_3_PRIORITY, 0, 0, TEST_FAULT_3_MAX, TEST_FAULT_3_MIN, &statusArray[85], 0, TEST_FAULT_3_MSG}, 
	(fault_attributes_t){false, false, TEST_FAULT_4_PRIORITY, 0, 0, TEST_FAULT_4_MAX, TEST_FAULT_4_MIN, &statusArray[86], 0, TEST_FAULT_4_MSG}, 
};
//END AUTO FAULT INFO ARRAY DEFS


//Corresponds to fault_defs.h
static uint8_t currentMCU;


//Variables containing the number of latched faults
static uint16_t fatalCount;
static uint16_t errorCount;
static uint16_t warnCount;
static uint16_t currCount;

static q_handle_t *q_tx;

uint16_t most_recent_latched;
q_handle_t q_fault_history;

//Variables containing the index/limits of owned faults (heartbeat)
static uint16_t ownedidx;
static uint16_t curridx;

//CAN msg ID
static uint32_t can_ext;

static bool fault_lib_disable;


/**
 * @brief Checks wheither fault should be latched or unlatched, updates
 *
 * @param id ID of fault to update
 * @param valueToCompare Current falue of fault. If conditions provided in JSON config are exceeded, fault will latch
 *
 * @return Whether function was successful
 */
bool setFault(int id, int valueToCompare) {
    //Fault Library disabled or the fault isn't owned by current MCU
    if (fault_lib_disable || GET_OWNER(id) != currentMCU)
    {
        return false;
    }
    uint16_t idx = GET_IDX(id);
    fault_attributes_t *fault = &faultArray[idx];

    //The fault is being forced to be a certain value, stop running
    if (fault->forceActive)
    {
        return statusArray[idx].latched;
    }
    //Templatch = result of comparison of limits + current value
    fault->tempLatch = ((valueToCompare >= fault->f_max) || (valueToCompare < fault->f_min)) ? true : false;
    updateFault(idx);
    return faultArray[idx].tempLatch;
}


/**
 * @brief Heartbeat messages for the various MCUs. Sends fault information, and should be scheduled
 *
 *
 * @return none
 */
void heartBeatTask() {
    if (ownedidx < 0 || fault_lib_disable) {
        return;
    }
        fault_status_t *status = &statusArray[curridx++];
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=can_ext, .DLC=3, .IDE=1};
        fault_can_format_t* data_a = (fault_can_format_t *) &msg.Data;
        data_a->fault_sync.idx = status->f_ID;
        data_a->fault_sync.latched = status->latched;
        qSendToBack(q_tx, &msg);
    //Move to the next fault in the owned array
     if ((curridx >= TOTAL_NUM_FAULTS) || (GET_OWNER(faultArray[curridx].status->f_ID) != currentMCU)) {
        curridx = ownedidx;
     }
}

/**
 * @brief Sends specified fault over CAN
 *
 * @param id ID of fault to send
 *
 * @return none
 */
static void txFaultSpecific(int id) {
    fault_status_t *status = &statusArray[GET_IDX(id)];
    CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=can_ext, .DLC=3, .IDE=1};
    fault_can_format_t* data_a = (fault_can_format_t *) &msg.Data;
    data_a->fault_sync.idx = status->f_ID;
    data_a->fault_sync.latched = status->latched;
    qSendToBack(q_tx, &msg);
}

//Function to update fault array from recieved messages
/**
 * @brief Function to update fault array from recieved fault status messages
 *
 * @param id ID of recieved fault
 * @param latched Fault state
 *
 * @return none
 */
void handleCallbacks(uint16_t id, bool latched) {
    fault_status_t recievedStatus = (fault_status_t){latched, id};
    fault_status_t *currStatus = &statusArray[GET_IDX(recievedStatus.f_ID)];
	if (recievedStatus.latched) {
        //If current Message = 0, and recieved message = 1 (fault is latching)
        if (!currStatus->latched) {
            currStatus->latched = recievedStatus.latched;
            most_recent_latched = GET_IDX(id);
            qSendToBack(&q_fault_history, &most_recent_latched);
            switch(faultArray[GET_IDX(recievedStatus.f_ID)].priority) {
                case FAULT_WARNING:
                    warnCount++;
                    break;
                case FAULT_ERROR:
                    errorCount++;
                    break;
                case FAULT_FATAL:
                    fatalCount++;
                    break;
            }
        }
    }
    //If current Message = 1, and recieved message = 0 (fault is unlatching)
    else {
        if (currStatus->latched) {
            currStatus->latched = recievedStatus.latched;
            switch(faultArray[GET_IDX(recievedStatus.f_ID)].priority) {
                case FAULT_WARNING:
                    warnCount--;
                    break;
                case FAULT_ERROR:
                    errorCount--;
                    break;
                case FAULT_FATAL:
                    fatalCount--;
                    break;
            }
        }
    }
}

/**
 * @brief Implement Force fault requests from DAQ
 *
 * @param id ID of fault to force
 * @param value State to set fault to
 *
 * @return none
 */
void set_fault_daq(uint16_t id, bool value) {
    if (GET_OWNER(id) == currentMCU) {
        forceFault(id, value);
    }
}

/**
 * @brief Return control back to this mcu from daq
 *
 * @param id ID of fault to return
 *
 * @return none
 */
void return_fault_control(uint16_t id) {
    if (GET_OWNER(id) == currentMCU) {
        unForce(id);
    }
}

/**
 * @brief Updates faults owned by current mcu
 *
 * @return none
 */
bool updateFault(uint16_t idx) {
    if (ownedidx < 0 || fault_lib_disable) {
        return false;
    }
    fault_attributes_t *fault = &faultArray[idx];
    uint32_t curr_time = sched.os_ticks;
    //Fault is showing up as latched
    if (((fault->tempLatch) && !(fault->status->latched))) {
        fault->time_since_latch = curr_time - fault->start_ticks;
        //Has latching period ended
        fault->status->latched = (fault->time_since_latch >= faultLatchTime[idx]) ? true : false;
        if (fault->status->latched) {
            fault->time_since_latch = 0;
            fault->start_ticks = curr_time;
            fault->bounces = 0;
            currCount++;
            most_recent_latched = idx;
            qSendToBack(&q_fault_history, &idx);
            switch(fault->priority) {
                case FAULT_WARNING:
                    warnCount++;
                    break;
                case FAULT_ERROR:
                    errorCount++;
                    break;
                case FAULT_FATAL:
                    fatalCount++;
                    break;
            }
            txFaultSpecific(idx);
        }
    }
    //Fault is showing up as unlatched
    else if (!(fault->tempLatch) && (fault->status->latched)) {
        fault->time_since_latch = curr_time - fault->start_ticks;
        //Make sure unlatch period has elapsed
        fault->status->latched = (fault->time_since_latch >= faultULatchTime[idx]) ? false : true;
        if (!(fault->status->latched)) {
            fault->time_since_latch = 0;
            fault->start_ticks = curr_time;
            fault->bounces = 0;
            currCount--;
            switch(fault->priority) {
                case FAULT_WARNING:
                    warnCount--;
                    break;
                case FAULT_ERROR:
                    errorCount--;
                    break;
                case FAULT_FATAL:
                    fatalCount--;
                    break;
            }
            txFaultSpecific(idx);
        }
    }
    else {
        // //Account for potential noise during the latching process
        // if (fault->time_since_latch > 0 && fault->bounces <= (uint16_t)(faultLatchTime[idx] * 0.4)&& fault->tempLatch == 1) {
        //     fault->time_since_latch = curr_time - fault->start_ticks;
        //     fault->bounces++;
        // }
        // else if (fault->time_since_latch > 0 && fault->bounces <= (uint16_t)(faultULatchTime[idx] * 0.4) && fault->tempLatch == 0) {
        //     fault->time_since_latch = curr_time = fault->start_ticks;
        //     fault->bounces++;
        // }
        // else if (fault->time_since_latch > 0 && fault->bounces > (uint16_t)(faultLatchTime[idx] * 0.4) && fault->tempLatch == 1) {
        //     fault->time_since_latch = 0;
        //     fault->start_ticks = curr_time;
        //     fault->status->latched = 1;
        //     fault->tempLatch = 1;
        //     fault->bounces = 0;
        //     currCount++;
        //     most_recent_latched = idx;
        //     switch(fault->priority) {
        //         case FAULT_WARNING:
        //             warnCount++;
        //             break;
        //         case FAULT_ERROR:
        //             errorCount++;
        //             break;
        //         case FAULT_FATAL:
        //             fatalCount++;
        //             break;
        //     }
        // }
        // else if (fault->time_since_latch > 0 && fault->bounces > (uint16_t)(faultULatchTime[idx] * 0.4) && fault->tempLatch == 0) {
        //     fault->time_since_latch = 0;
        //     fault->start_ticks = curr_time;
        //     fault->status->latched = 1;
        //     fault->tempLatch = 1;
        //     fault->bounces = 0;
        //     currCount++;
        //     most_recent_latched = idx;
        //     switch(fault->priority) {
        //         case FAULT_WARNING:
        //             warnCount++;
        //             break;
        //         case FAULT_ERROR:
        //             errorCount++;
        //             break;
        //         case FAULT_FATAL:
        //             fatalCount++;
        //             break;
        //     }
        // }
        // else {
            fault->time_since_latch = 0;
            fault->start_ticks = curr_time;
        // }
    }
}


/**
 * @brief Disables Fault Library
 *
 * @return none
 */
void killFaultLibrary() {
    fault_lib_disable = true;
}

/**
 * @brief Checks if current mcu has faults
 *
 * @return Whether any faults on current mcu have latched
 */
bool currMCULatched() {
    return (currCount == 0) ? false : true;
}

/**
 * @brief Checks if any warning level faults have latched
 *
 * @return Whether any warning level faults have latched
 */
bool warningLatched() {
    return (warnCount == 0) ? false : true;
}

/**
 * @brief Checks if any error level faults have latched
 *
 * @return Whether any error level faults have latched
 */
bool errorLatched() {
    return (errorCount == 0) ? false : true;
}

/**
 * @brief Checks if any fatal level faults have latched
 *
 * @return Whether any fatal level faults have latched
 */
bool fatalLatched() {
    return (fatalCount == 0) ? false : true;
}

/**
 * @brief Checks if the other MCUs have faults latched
 *
 * @return Whether other MCUs have latched
 */
bool otherMCUsLatched() {
    return (warnCount + errorCount + fatalCount - currCount == 0) ? false : true;
}

/**
 * @brief Checks if any faults have latched, regardless of mcu
 *
 * @return Whether any fault has latched
 */
bool isLatched() {
    return (warnCount + errorCount + fatalCount == 0) ? false : true;
}

/**
 * @brief Checks a specific fault for its status
 *
 * @param id The id of the fault to check
 *
 * @return Whether requested fault is latched
 */
bool checkFault(int id) {
    return statusArray[GET_IDX(id)].latched;
}

/**
 * @brief Allow fault to be controlled by updateFaults()
 *
 * @param id ID of fault to unforce
 *
 * @return none
 */
static void unForce(int id) {
    faultArray[GET_IDX(id)].forceActive = false;
}



/**
 * @brief Set a fault to a certain state, regardless of recieved data - disables updateFaults()
 *
 * @param id ID of fault to force
 * @param state State to force fault to
 *
 * @return none
 */
static void forceFault(int id, bool state) {
    uint16_t idx = GET_IDX(id);
    //If it is forced to be latched and wasn't already
    if (state & !statusArray[idx].latched) {
        currCount++;
        most_recent_latched = idx;
        qSendToBack(&q_fault_history, &idx);
        switch(faultArray[idx].priority) {
            case FAULT_WARNING:
                warnCount++;
                break;
            case FAULT_ERROR:
                errorCount++;
                break;
            case FAULT_FATAL:
                fatalCount++;
                break;
        }
    }
    //If it is forced to be unlatched and was latched before
    else if (!state & statusArray[idx].latched) {
       currCount--;
        switch(faultArray[idx].priority) {
            case FAULT_WARNING:
                warnCount--;
                break;
            case FAULT_ERROR:
                errorCount--;
                break;
            case FAULT_FATAL:
                fatalCount--;
                break;
        }
    }
    //Update the array and send through CAN
    faultArray[idx].tempLatch = state;
    faultArray[idx].forceActive = true;
    faultArray[idx].time_since_latch = 0;
    faultArray[idx].bounces = 0;
    faultArray[idx].start_ticks = sched.os_ticks;
    statusArray[idx].latched = state;
	txFaultSpecific(id);

}


/**
 * @brief Initialize the Fault library with starting values
 *
 * @param mcu Current MCU
 * @param qxQ Pointer to CAN TX queue
 * @param ext CAN ext ID from module's can_parse.h, usually of format ID_FAULT_SYNC{mcu}
 *
 * @return none
 */
void initFaultLibrary(uint8_t mcu, q_handle_t* txQ, uint32_t ext) {
    most_recent_latched = 0xFFFF;
    fault_lib_disable = false;
    bool foundStartIdx = false;
    can_ext = ext;
    q_tx = txQ;
    currentMCU = mcu;
    uint16_t num_owned_faults = 0;
    uint16_t num_recieved_faults = 0;
    //Find the beginning of current mcu's faults, if a fault exists
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
        if (GET_OWNER(faultArray[i].status->f_ID) == mcu && !foundStartIdx) {
            foundStartIdx = true;
            ownedidx = i;
        }
    }
    if (!foundStartIdx) {
        ownedidx = -1;
    }
    curridx = ownedidx;
    warnCount = 0;
    errorCount = 0;
    fatalCount = 0;
    currCount = 0;
    qConstruct(&q_fault_history, sizeof(uint16_t));
}