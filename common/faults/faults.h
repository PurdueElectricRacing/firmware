/**
 * @file faults.h
 * @author Aditya Anand (anand89@purdue.edu)
 * @brief Creating a library of faults to create an easy to debug atmosphere on the car
 * @version 0.1
 * @date 2022-05-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _FAULTS_H_
#define _FAULTS_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "common/queue/queue.h"

#define MAX_MSG_SIZE 75

//BEGIN AUTO TOTAL DEFS
#define TOTAL_MAIN_MODULE_FAULTS 9
#define TOTAL_DRIVELINE_FRONT_FAULTS 4
#define TOTAL_DASHBOARD_FAULTS 6
#define TOTAL_PRECHARGE_FAULTS 28
#define TOTAL_TV_FAULTS 1
#define TOTAL_TEST_FAULTS 4
#define TOTAL_MCU_NUM 6
#define TOTAL_NUM_FAULTS 52
//END AUTO TOTAL DEFS

//BEGIN AUTO ID DEFS
#define ID_PCHG_IMPLAUS_FAULT 0x0
#define ID_RTD_EXIT_FAULT 0x1
#define ID_LEFT_MC_CONN_FAULT 0x2
#define ID_RIGHT_MC_CONN_FAULT 0x3
#define ID_MCU_TEMP_HIGH_FAULT 0x4
#define ID_LV_BAT_LOW_FAULT 0x5
#define ID_LV_BAT_VERY_LOW_FAULT 0x6
#define ID_LV_BAT_BMS_FAULT 0x7
#define ID_DRIVE_FLOW_FAULT 0x8
#define ID_MOT_FRONT_OT_FAULT 0x1009
#define ID_WLSPD_L_FAULT 0x100a
#define ID_WLSPD_R_FAULT 0x100b
#define ID_DRIVELINE_COMM_FAULT 0x100c
#define ID_APPS_WIRING_T1_FAULT 0x200d
#define ID_APPS_WIRING_T2_FAULT 0x200e
#define ID_BSE_FAULT 0x200f
#define ID_BSPD_FAULT 0x2010
#define ID_IMPLAUS_DETECTED_FAULT 0x2011
#define ID_APPS_BRAKE_FAULT 0x2012
#define ID_DISCHARGE_LIMIT_ENFORCE_FAULT 0x3013
#define ID_CHARGER_SAFETY_RELAY_FAULT 0x3014
#define ID_INTERNAL_HARDWARE_FAULT 0x3015
#define ID_HEATSINK_THERMISTOR_FAULT 0x3016
#define ID_SOFTWARE_FAULT 0x3017
#define ID_MAX_CELLV_HIGH_FAULT 0x3018
#define ID_MIN_CELLV_LOW_FAULT 0x3019
#define ID_PACK_OVERHEAT_ORION_FAULT 0x301a
#define ID_INTERNAL_COMMS_FAULT 0x301b
#define ID_CELL_BALANCING_FOFF_FAULT 0x301c
#define ID_WEAK_CELL_FAULT 0x301d
#define ID_LOW_CELLV_FAULT 0x301e
#define ID_OPEN_WIRE_FAULT 0x301f
#define ID_CURRENT_SENSOR_FAULT 0x3020
#define ID_MAX_CELLV_O5V_FAULT 0x3021
#define ID_CELL_ASIC_FAULT 0x3022
#define ID_WEAK_PACK_FAULT 0x3023
#define ID_FAN_MONITOR_FAULT 0x3024
#define ID_THERMISTOR_FAULT 0x3025
#define ID_EXTERNAL_COMMS_FAULT 0x3026
#define ID_REDUNDANT_PSU_FAULT 0x3027
#define ID_HV_ISOLATION_FAULT 0x3028
#define ID_INPUT_PSU_FAULT 0x3029
#define ID_CHARGE_LIMIT_ENFORCE_FAULT 0x302a
#define ID_PACK_TEMP_FAULT 0x302b
#define ID_PACK_TEMP_EXCEEDED_FAULT 0x302c
#define ID_MIN_PACK_TEMP_FAULT 0x302d
#define ID_IMD_FAULT 0x302e
#define ID_TV_OFFLINE_FAULT 0x402f
#define ID_TEST_FAULT_1_FAULT 0x5030
#define ID_TEST_FAULT_2_FAULT 0x5031
#define ID_TEST_FAULT_3_FAULT 0x5032
#define ID_TEST_FAULT_4_FAULT 0x5033
//END AUTO ID DEFS

//Macro defs for accessing aspects of id
#define GET_IDX(id) (id & 0x0FFF)
#define GET_OWNER(id) ((id & 0xF000) >> 12)


//WARNING: Doesn't affect driving state (Car can still safely drive)
//CRITICAL: Car exits ready2drive, but LV + HV systems still active
//FATAL: The Car SDC is activated
//BEGIN AUTO PRIORITY DEFS
#define PCHG_IMPLAUS_PRIORITY 1
#define RTD_EXIT_PRIORITY 0
#define LEFT_MC_CONN_PRIORITY 1
#define RIGHT_MC_CONN_PRIORITY 1
#define MCU_TEMP_HIGH_PRIORITY 0
#define LV_BAT_LOW_PRIORITY 0
#define LV_BAT_VERY_LOW_PRIORITY 1
#define LV_BAT_BMS_PRIORITY 1
#define DRIVE_FLOW_PRIORITY 1
#define MOT_FRONT_OT_PRIORITY 1
#define WLSPD_L_PRIORITY 0
#define WLSPD_R_PRIORITY 0
#define DRIVELINE_COMM_PRIORITY 2
#define APPS_WIRING_T1_PRIORITY 1
#define APPS_WIRING_T2_PRIORITY 1
#define BSE_PRIORITY 1
#define BSPD_PRIORITY 1
#define IMPLAUS_DETECTED_PRIORITY 1
#define APPS_BRAKE_PRIORITY 1
#define DISCHARGE_LIMIT_ENFORCE_PRIORITY 2
#define CHARGER_SAFETY_RELAY_PRIORITY 2
#define INTERNAL_HARDWARE_PRIORITY 2
#define HEATSINK_THERMISTOR_PRIORITY 2
#define SOFTWARE_PRIORITY 2
#define MAX_CELLV_HIGH_PRIORITY 2
#define MIN_CELLV_LOW_PRIORITY 2
#define PACK_OVERHEAT_ORION_PRIORITY 2
#define INTERNAL_COMMS_PRIORITY 2
#define CELL_BALANCING_FOFF_PRIORITY 2
#define WEAK_CELL_PRIORITY 2
#define LOW_CELLV_PRIORITY 2
#define OPEN_WIRE_PRIORITY 2
#define CURRENT_SENSOR_PRIORITY 2
#define MAX_CELLV_O5V_PRIORITY 2
#define CELL_ASIC_PRIORITY 2
#define WEAK_PACK_PRIORITY 2
#define FAN_MONITOR_PRIORITY 2
#define THERMISTOR_PRIORITY 2
#define EXTERNAL_COMMS_PRIORITY 2
#define REDUNDANT_PSU_PRIORITY 2
#define HV_ISOLATION_PRIORITY 2
#define INPUT_PSU_PRIORITY 2
#define CHARGE_LIMIT_ENFORCE_PRIORITY 2
#define PACK_TEMP_PRIORITY 0
#define PACK_TEMP_EXCEEDED_PRIORITY 1
#define MIN_PACK_TEMP_PRIORITY 1
#define IMD_PRIORITY 2
#define TV_OFFLINE_PRIORITY 0
#define TEST_FAULT_1_PRIORITY 0
#define TEST_FAULT_2_PRIORITY 1
#define TEST_FAULT_3_PRIORITY 2
#define TEST_FAULT_4_PRIORITY 0
//END AUTO PRIORITY DEFS

//BEGIN AUTO MAX DEFS
#define PCHG_IMPLAUS_MAX 1
#define RTD_EXIT_MAX 1
#define LEFT_MC_CONN_MAX 1
#define RIGHT_MC_CONN_MAX 1
#define MCU_TEMP_HIGH_MAX 50
#define LV_BAT_LOW_MAX 34
#define LV_BAT_VERY_LOW_MAX 34
#define LV_BAT_BMS_MAX 1
#define DRIVE_FLOW_MAX 63
#define MOT_FRONT_OT_MAX 100
#define WLSPD_L_MAX 4096
#define WLSPD_R_MAX 4096
#define DRIVELINE_COMM_MAX 1
#define APPS_WIRING_T1_MAX 3000
#define APPS_WIRING_T2_MAX 3000
#define BSE_MAX 1
#define BSPD_MAX 182
#define IMPLAUS_DETECTED_MAX 700
#define APPS_BRAKE_MAX 1
#define DISCHARGE_LIMIT_ENFORCE_MAX 1
#define CHARGER_SAFETY_RELAY_MAX 1
#define INTERNAL_HARDWARE_MAX 1
#define HEATSINK_THERMISTOR_MAX 1
#define SOFTWARE_MAX 1
#define MAX_CELLV_HIGH_MAX 1
#define MIN_CELLV_LOW_MAX 1
#define PACK_OVERHEAT_ORION_MAX 1
#define INTERNAL_COMMS_MAX 1
#define CELL_BALANCING_FOFF_MAX 1
#define WEAK_CELL_MAX 1
#define LOW_CELLV_MAX 1
#define OPEN_WIRE_MAX 1
#define CURRENT_SENSOR_MAX 1
#define MAX_CELLV_O5V_MAX 1
#define CELL_ASIC_MAX 1
#define WEAK_PACK_MAX 1
#define FAN_MONITOR_MAX 1
#define THERMISTOR_MAX 1
#define EXTERNAL_COMMS_MAX 1
#define REDUNDANT_PSU_MAX 1
#define HV_ISOLATION_MAX 1
#define INPUT_PSU_MAX 1
#define CHARGE_LIMIT_ENFORCE_MAX 1
#define PACK_TEMP_MAX 5000
#define PACK_TEMP_EXCEEDED_MAX 6000
#define MIN_PACK_TEMP_MAX 10000
#define IMD_MAX 1
#define TV_OFFLINE_MAX 1
#define TEST_FAULT_1_MAX 1
#define TEST_FAULT_2_MAX 1
#define TEST_FAULT_3_MAX 1
#define TEST_FAULT_4_MAX 123
//END AUTO MAX DEFS

//BEGIN AUTO MIN DEFS
#define PCHG_IMPLAUS_MIN 0
#define RTD_EXIT_MIN 0
#define LEFT_MC_CONN_MIN 0
#define RIGHT_MC_CONN_MIN 0
#define MCU_TEMP_HIGH_MIN 0
#define LV_BAT_LOW_MIN 24
#define LV_BAT_VERY_LOW_MIN 22
#define LV_BAT_BMS_MIN 0
#define DRIVE_FLOW_MIN 46
#define MOT_FRONT_OT_MIN 10
#define WLSPD_L_MIN 0
#define WLSPD_R_MIN 0
#define DRIVELINE_COMM_MIN 0
#define APPS_WIRING_T1_MIN 200
#define APPS_WIRING_T2_MIN 200
#define BSE_MIN 0
#define BSPD_MIN 0
#define IMPLAUS_DETECTED_MIN 0
#define APPS_BRAKE_MIN 0
#define DISCHARGE_LIMIT_ENFORCE_MIN 0
#define CHARGER_SAFETY_RELAY_MIN 0
#define INTERNAL_HARDWARE_MIN 0
#define HEATSINK_THERMISTOR_MIN 0
#define SOFTWARE_MIN 0
#define MAX_CELLV_HIGH_MIN 0
#define MIN_CELLV_LOW_MIN 0
#define PACK_OVERHEAT_ORION_MIN 0
#define INTERNAL_COMMS_MIN 0
#define CELL_BALANCING_FOFF_MIN 0
#define WEAK_CELL_MIN 0
#define LOW_CELLV_MIN 0
#define OPEN_WIRE_MIN 0
#define CURRENT_SENSOR_MIN 0
#define MAX_CELLV_O5V_MIN 0
#define CELL_ASIC_MIN 0
#define WEAK_PACK_MIN 0
#define FAN_MONITOR_MIN 0
#define THERMISTOR_MIN 0
#define EXTERNAL_COMMS_MIN 0
#define REDUNDANT_PSU_MIN 0
#define HV_ISOLATION_MIN 0
#define INPUT_PSU_MIN 0
#define CHARGE_LIMIT_ENFORCE_MIN 0
#define PACK_TEMP_MIN 0
#define PACK_TEMP_EXCEEDED_MIN 0
#define MIN_PACK_TEMP_MIN 10
#define IMD_MIN 0
#define TV_OFFLINE_MIN 0
#define TEST_FAULT_1_MIN 0
#define TEST_FAULT_2_MIN 0
#define TEST_FAULT_3_MIN 0
#define TEST_FAULT_4_MIN 5
//END AUTO MIN DEFS

//BEGIN AUTO LATCH DEFS
#define PCHG_IMPLAUS_LATCH_TIME 50
#define RTD_EXIT_LATCH_TIME 100
#define LEFT_MC_CONN_LATCH_TIME 3000
#define RIGHT_MC_CONN_LATCH_TIME 3000
#define MCU_TEMP_HIGH_LATCH_TIME 1000
#define LV_BAT_LOW_LATCH_TIME 1000
#define LV_BAT_VERY_LOW_LATCH_TIME 1000
#define LV_BAT_BMS_LATCH_TIME 50
#define DRIVE_FLOW_LATCH_TIME 10
#define MOT_FRONT_OT_LATCH_TIME 10
#define WLSPD_L_LATCH_TIME 20
#define WLSPD_R_LATCH_TIME 10
#define DRIVELINE_COMM_LATCH_TIME 10
#define APPS_WIRING_T1_LATCH_TIME 10
#define APPS_WIRING_T2_LATCH_TIME 10
#define BSE_LATCH_TIME 100
#define BSPD_LATCH_TIME 100
#define IMPLAUS_DETECTED_LATCH_TIME 10
#define APPS_BRAKE_LATCH_TIME 10
#define DISCHARGE_LIMIT_ENFORCE_LATCH_TIME 5
#define CHARGER_SAFETY_RELAY_LATCH_TIME 5
#define INTERNAL_HARDWARE_LATCH_TIME 5
#define HEATSINK_THERMISTOR_LATCH_TIME 5
#define SOFTWARE_LATCH_TIME 5
#define MAX_CELLV_HIGH_LATCH_TIME 5
#define MIN_CELLV_LOW_LATCH_TIME 5
#define PACK_OVERHEAT_ORION_LATCH_TIME 5
#define INTERNAL_COMMS_LATCH_TIME 5
#define CELL_BALANCING_FOFF_LATCH_TIME 5
#define WEAK_CELL_LATCH_TIME 5
#define LOW_CELLV_LATCH_TIME 5
#define OPEN_WIRE_LATCH_TIME 5
#define CURRENT_SENSOR_LATCH_TIME 5
#define MAX_CELLV_O5V_LATCH_TIME 5
#define CELL_ASIC_LATCH_TIME 5
#define WEAK_PACK_LATCH_TIME 5
#define FAN_MONITOR_LATCH_TIME 5
#define THERMISTOR_LATCH_TIME 5
#define EXTERNAL_COMMS_LATCH_TIME 5
#define REDUNDANT_PSU_LATCH_TIME 5
#define HV_ISOLATION_LATCH_TIME 5
#define INPUT_PSU_LATCH_TIME 5
#define CHARGE_LIMIT_ENFORCE_LATCH_TIME 5
#define PACK_TEMP_LATCH_TIME 2000
#define PACK_TEMP_EXCEEDED_LATCH_TIME 2000
#define MIN_PACK_TEMP_LATCH_TIME 2000
#define IMD_LATCH_TIME 100
#define TV_OFFLINE_LATCH_TIME 10
#define TEST_FAULT_1_LATCH_TIME 10
#define TEST_FAULT_2_LATCH_TIME 10
#define TEST_FAULT_3_LATCH_TIME 10
#define TEST_FAULT_4_LATCH_TIME 10
//END AUTO LATCH DEFS

//BEGIN AUTO UNLATCH DEFS
#define PCHG_IMPLAUS_UNLATCH_TIME 1000
#define RTD_EXIT_UNLATCH_TIME 1000
#define LEFT_MC_CONN_UNLATCH_TIME 1000
#define RIGHT_MC_CONN_UNLATCH_TIME 1000
#define MCU_TEMP_HIGH_UNLATCH_TIME 2000
#define LV_BAT_LOW_UNLATCH_TIME 2000
#define LV_BAT_VERY_LOW_UNLATCH_TIME 2000
#define LV_BAT_BMS_UNLATCH_TIME 2000
#define DRIVE_FLOW_UNLATCH_TIME 10
#define MOT_FRONT_OT_UNLATCH_TIME 10
#define WLSPD_L_UNLATCH_TIME 20
#define WLSPD_R_UNLATCH_TIME 10
#define DRIVELINE_COMM_UNLATCH_TIME 10
#define APPS_WIRING_T1_UNLATCH_TIME 1000
#define APPS_WIRING_T2_UNLATCH_TIME 1000
#define BSE_UNLATCH_TIME 1000
#define BSPD_UNLATCH_TIME 1000
#define IMPLAUS_DETECTED_UNLATCH_TIME 100
#define APPS_BRAKE_UNLATCH_TIME 1000
#define DISCHARGE_LIMIT_ENFORCE_UNLATCH_TIME 5
#define CHARGER_SAFETY_RELAY_UNLATCH_TIME 5
#define INTERNAL_HARDWARE_UNLATCH_TIME 5
#define HEATSINK_THERMISTOR_UNLATCH_TIME 5
#define SOFTWARE_UNLATCH_TIME 5
#define MAX_CELLV_HIGH_UNLATCH_TIME 5
#define MIN_CELLV_LOW_UNLATCH_TIME 5
#define PACK_OVERHEAT_ORION_UNLATCH_TIME 5
#define INTERNAL_COMMS_UNLATCH_TIME 5
#define CELL_BALANCING_FOFF_UNLATCH_TIME 5
#define WEAK_CELL_UNLATCH_TIME 5
#define LOW_CELLV_UNLATCH_TIME 5
#define OPEN_WIRE_UNLATCH_TIME 5
#define CURRENT_SENSOR_UNLATCH_TIME 5
#define MAX_CELLV_O5V_UNLATCH_TIME 5
#define CELL_ASIC_UNLATCH_TIME 5
#define WEAK_PACK_UNLATCH_TIME 5
#define FAN_MONITOR_UNLATCH_TIME 5
#define THERMISTOR_UNLATCH_TIME 5
#define EXTERNAL_COMMS_UNLATCH_TIME 5
#define REDUNDANT_PSU_UNLATCH_TIME 5
#define HV_ISOLATION_UNLATCH_TIME 5
#define INPUT_PSU_UNLATCH_TIME 5
#define CHARGE_LIMIT_ENFORCE_UNLATCH_TIME 5
#define PACK_TEMP_UNLATCH_TIME 5000
#define PACK_TEMP_EXCEEDED_UNLATCH_TIME 10000
#define MIN_PACK_TEMP_UNLATCH_TIME 10000
#define IMD_UNLATCH_TIME 5000
#define TV_OFFLINE_UNLATCH_TIME 10
#define TEST_FAULT_1_UNLATCH_TIME 10
#define TEST_FAULT_2_UNLATCH_TIME 10
#define TEST_FAULT_3_UNLATCH_TIME 10
#define TEST_FAULT_4_UNLATCH_TIME 10
//END AUTO UNLATCH DEFS

//BEGIN AUTO SCREENMSG DEFS
#define PCHG_IMPLAUS_MSG "Precharge Implausibility\0" 
#define RTD_EXIT_MSG "HV not detected, idling\0" 
#define LEFT_MC_CONN_MSG "LEFT MC CONN FAIL\0" 
#define RIGHT_MC_CONN_MSG "RIGHT MC CONN FAIL\0" 
#define MCU_TEMP_HIGH_MSG "HIGH PDU MCU TEMP\0" 
#define LV_BAT_LOW_MSG "LV Bat Getting Low\0" 
#define LV_BAT_VERY_LOW_MSG "LV Bat Very Low\0" 
#define LV_BAT_BMS_MSG "LV Bat BMS Fault\0" 
#define DRIVE_FLOW_MSG "battery pump flow brokey\0" 
#define MOT_FRONT_OT_MSG "Front motors overheating\0" 
#define WLSPD_L_MSG "wheelspeeed L broken\0" 
#define WLSPD_R_MSG "wheelspeeed R broken\0" 
#define DRIVELINE_COMM_MSG "Driveline offline\0" 
#define APPS_WIRING_T1_MSG "APPS Wiring Fail T1\0" 
#define APPS_WIRING_T2_MSG "APPS Wiring Fail T2\0" 
#define BSE_MSG "Brake Wiring Fail (BSE)\0" 
#define BSPD_MSG "BSE Wiring Fail B2\0" 
#define IMPLAUS_DETECTED_MSG "APPS Implaus Detected\0" 
#define APPS_BRAKE_MSG "APPS Brake Fault\0" 
#define DISCHARGE_LIMIT_ENFORCE_MSG "Orion Discharge Limit\0" 
#define CHARGER_SAFETY_RELAY_MSG "Orion Charger Safety Error\0" 
#define INTERNAL_HARDWARE_MSG "Orion Internal Fault\0" 
#define HEATSINK_THERMISTOR_MSG "Orion Overheating\0" 
#define SOFTWARE_MSG "Orion Software Error\0" 
#define MAX_CELLV_HIGH_MSG "Max Cell Volts too High\0" 
#define MIN_CELLV_LOW_MSG "Min Cell Volts too Low\0" 
#define PACK_OVERHEAT_ORION_MSG "Orion Pack Overheat Fault\0" 
#define INTERNAL_COMMS_MSG "Orion Internal Comms Error\0" 
#define CELL_BALANCING_FOFF_MSG "Cell Balancing Offline\0" 
#define WEAK_CELL_MSG "Weak Cell Fault\0" 
#define LOW_CELLV_MSG "Low Cell Voltage Fault\0" 
#define OPEN_WIRE_MSG "Orion Open Wire Fault\0" 
#define CURRENT_SENSOR_MSG "Orion Current Sensor Fault\0" 
#define MAX_CELLV_O5V_MSG "Max CellV > 5\0" 
#define CELL_ASIC_MSG "Orion Cell ASIC\0" 
#define WEAK_PACK_MSG "Orion Weak Pack Fault\0" 
#define FAN_MONITOR_MSG "Orion Fan Monitor Fault\0" 
#define THERMISTOR_MSG "Orion Thermistor Fault\0" 
#define EXTERNAL_COMMS_MSG "Orion External Communication Fault\0" 
#define REDUNDANT_PSU_MSG "Orion Redundant PSU Found\0" 
#define HV_ISOLATION_MSG "Orion HV Isolation Fault\0" 
#define INPUT_PSU_MSG "Orion Input PSU Fault\0" 
#define CHARGE_LIMIT_ENFORCE_MSG "Orion Charge Limit\0" 
#define PACK_TEMP_MSG "Pack Temp High (> 50)\0" 
#define PACK_TEMP_EXCEEDED_MSG "Pack Overheating\0" 
#define MIN_PACK_TEMP_MSG "Pack Minimum temp < 10\0" 
#define IMD_MSG "IMD Isolation Fault\0" 
#define TV_OFFLINE_MSG "TV offline\0" 
#define TEST_FAULT_1_MSG "Test fault 1\0" 
#define TEST_FAULT_2_MSG "Test fault 2\0" 
#define TEST_FAULT_3_MSG "Test fault 3\0" 
#define TEST_FAULT_4_MSG "Test fault 4\0" 
//END AUTO SCREENMSG DEFS

extern uint16_t most_recent_latched;

typedef enum {
    FAULT_WARNING = 0,
    FAULT_ERROR = 1,
    FAULT_FATAL = 2
} fault_priority_t;



//Contains info about fault state
typedef struct {
    bool latched;
    int f_ID;
} fault_status_t;

//Contains info about the fault as a whole
typedef struct {
    bool tempLatch;
    bool forceActive;
    fault_priority_t priority;
    uint8_t bounces;
    uint16_t time_since_latch;
    int f_max;
    int f_min;
    fault_status_t *status;
    uint32_t start_ticks;
    char* screen_MSG;
} fault_attributes_t;

extern fault_attributes_t faultArray[TOTAL_NUM_FAULTS];

//Union to package CAN messages
typedef union {
    struct {
        uint64_t idx: 16;
        uint64_t latched: 1;
    } fault_sync;
    uint8_t raw_data[8];
} __attribute__((packed)) fault_can_format_t;




//Vars
extern fault_status_t message[TOTAL_NUM_FAULTS];
extern fault_attributes_t attributes[TOTAL_NUM_FAULTS];

//Function defs
void initFaultLibrary(uint8_t mcu, q_handle_t* txQ, uint32_t ext);
bool setFault(int, int);
static void forceFault(int id, bool state);
static void unForce(int);
static void txFaultSpecific(int);
bool updateFault(uint16_t idx);
void heartBeatTask();
void updateFaults();
void killFaultLibrary();
void handleCallbacks(uint16_t id, bool latched);
bool currMCULatched();
bool warningLatched();
bool errorLatched();
bool fatalLatched();
bool otherMCUsLatched();
bool isLatched();
bool checkFault(int id);

#endif