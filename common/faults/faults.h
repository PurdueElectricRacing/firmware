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
#define TOTAL_MAIN_MODULE_FAULTS 4
#define TOTAL_DRIVELINE_FRONT_FAULTS 4
#define TOTAL_DASHBOARD_FAULTS 1
#define TOTAL_PRECHARGE_FAULTS 1
#define TOTAL_TV_FAULTS 1
#define TOTAL_TEST_FAULTS 4
#define TOTAL_MCU_NUM 6
#define TOTAL_NUM_FAULTS 15
//END AUTO TOTAL DEFS

//BEGIN AUTO ID DEFS
#define ID_BATT_FLOW_FAULT 0x0
#define ID_DRIVE_FLOW_FAULT 0x1
#define ID_MAIN_COMM_FAULT 0x2
#define ID_LV_DEAD_FAULT 0x3
#define ID_MOT_FRONT_OT_FAULT 0x1004
#define ID_WLSPD_L_FAULT 0x1005
#define ID_WLSPD_R_FAULT 0x1006
#define ID_DRIVELINE_COMM_FAULT 0x1007
#define ID_BMS_FAULT 0x2008
#define ID_BATT_OT_FAULT 0x3009
#define ID_TV_OFFLINE_FAULT 0x400a
#define ID_TEST_FAULT_1_FAULT 0x500b
#define ID_TEST_FAULT_2_FAULT 0x500c
#define ID_TEST_FAULT_3_FAULT 0x500d
#define ID_TEST_FAULT_4_FAULT 0x500e
//END AUTO ID DEFS

//Macro defs for accessing aspects of id
#define GET_IDX(id) (id & 0x0FFF)
#define GET_OWNER(id) ((id & 0xF000) >> 12)


//WARNING: Doesn't affect driving state (Car can still safely drive)
//CRITICAL: Car exits ready2drive, but LV + HV systems still active
//FATAL: The Car SDC is activated
//BEGIN AUTO PRIORITY DEFS
#define BATT_FLOW_PRIORITY 1
#define DRIVE_FLOW_PRIORITY 1
#define MAIN_COMM_PRIORITY 2
#define LV_DEAD_PRIORITY 2
#define MOT_FRONT_OT_PRIORITY 1
#define WLSPD_L_PRIORITY 0
#define WLSPD_R_PRIORITY 0
#define DRIVELINE_COMM_PRIORITY 2
#define BMS_PRIORITY 2
#define BATT_OT_PRIORITY 1
#define TV_OFFLINE_PRIORITY 0
#define TEST_FAULT_1_PRIORITY 0
#define TEST_FAULT_2_PRIORITY 1
#define TEST_FAULT_3_PRIORITY 2
#define TEST_FAULT_4_PRIORITY 0
//END AUTO PRIORITY DEFS

//BEGIN AUTO MAX DEFS
#define BATT_FLOW_MAX 63
#define DRIVE_FLOW_MAX 63
#define MAIN_COMM_MAX 1
#define LV_DEAD_MAX 3200
#define MOT_FRONT_OT_MAX 100
#define WLSPD_L_MAX 4096
#define WLSPD_R_MAX 4096
#define DRIVELINE_COMM_MAX 1
#define BMS_MAX 1
#define BATT_OT_MAX 50
#define TV_OFFLINE_MAX 1
#define TEST_FAULT_1_MAX 1
#define TEST_FAULT_2_MAX 1
#define TEST_FAULT_3_MAX 1
#define TEST_FAULT_4_MAX 123
//END AUTO MAX DEFS

//BEGIN AUTO MIN DEFS
#define BATT_FLOW_MIN 46
#define DRIVE_FLOW_MIN 46
#define MAIN_COMM_MIN 0
#define LV_DEAD_MIN 2000
#define MOT_FRONT_OT_MIN 10
#define WLSPD_L_MIN 0
#define WLSPD_R_MIN 0
#define DRIVELINE_COMM_MIN 0
#define BMS_MIN 0
#define BATT_OT_MIN 10
#define TV_OFFLINE_MIN 0
#define TEST_FAULT_1_MIN 0
#define TEST_FAULT_2_MIN 0
#define TEST_FAULT_3_MIN 0
#define TEST_FAULT_4_MIN 5
//END AUTO MIN DEFS

//BEGIN AUTO LATCH DEFS
#define BATT_FLOW_LATCH_TIME 10
#define DRIVE_FLOW_LATCH_TIME 10
#define MAIN_COMM_LATCH_TIME 10
#define LV_DEAD_LATCH_TIME 10
#define MOT_FRONT_OT_LATCH_TIME 10
#define WLSPD_L_LATCH_TIME 20
#define WLSPD_R_LATCH_TIME 10
#define DRIVELINE_COMM_LATCH_TIME 10
#define BMS_LATCH_TIME 10
#define BATT_OT_LATCH_TIME 10
#define TV_OFFLINE_LATCH_TIME 10
#define TEST_FAULT_1_LATCH_TIME 10
#define TEST_FAULT_2_LATCH_TIME 10
#define TEST_FAULT_3_LATCH_TIME 10
#define TEST_FAULT_4_LATCH_TIME 10
//END AUTO LATCH DEFS

//BEGIN AUTO UNLATCH DEFS
#define BATT_FLOW_UNLATCH_TIME 10
#define DRIVE_FLOW_UNLATCH_TIME 10
#define MAIN_COMM_UNLATCH_TIME 10
#define LV_DEAD_UNLATCH_TIME 10
#define MOT_FRONT_OT_UNLATCH_TIME 10
#define WLSPD_L_UNLATCH_TIME 20
#define WLSPD_R_UNLATCH_TIME 10
#define DRIVELINE_COMM_UNLATCH_TIME 10
#define BMS_UNLATCH_TIME 10
#define BATT_OT_UNLATCH_TIME 10
#define TV_OFFLINE_UNLATCH_TIME 10
#define TEST_FAULT_1_UNLATCH_TIME 10
#define TEST_FAULT_2_UNLATCH_TIME 10
#define TEST_FAULT_3_UNLATCH_TIME 10
#define TEST_FAULT_4_UNLATCH_TIME 10
//END AUTO UNLATCH DEFS

//BEGIN AUTO SCREENMSG DEFS
#define BATT_FLOW_MSG "battery pump flow brokey\0" 
#define DRIVE_FLOW_MSG "battery pump flow brokey\0" 
#define MAIN_COMM_MSG "main not talking\0" 
#define LV_DEAD_MSG "LV dead, shut car off ASAP\0" 
#define MOT_FRONT_OT_MSG "Front motors overheating\0" 
#define WLSPD_L_MSG "wheelspeeed L broken\0" 
#define WLSPD_R_MSG "wheelspeeed R broken\0" 
#define DRIVELINE_COMM_MSG "Driveline offline\0" 
#define BMS_MSG "BMS fault\0" 
#define BATT_OT_MSG "Battery overheating\0" 
#define TV_OFFLINE_MSG "TV offline\0" 
#define TEST_FAULT_1_MSG "Test fault 1\0" 
#define TEST_FAULT_2_MSG "Test fault 2\0" 
#define TEST_FAULT_3_MSG "Test fault 3\0" 
#define TEST_FAULT_4_MSG "Test fault 4\0" 
//END AUTO SCREENMSG DEFS



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
    char* screen_MSG;
} fault_attributes_t;




//Vars
extern fault_status_t message[TOTAL_NUM_FAULTS];
extern fault_attributes_t attributes[TOTAL_NUM_FAULTS];

//Function defs
void initFaultLibrary(uint8_t, q_handle_t*, q_handle_t*);
bool setFault(int, int);
void forceFault(int, bool);
void unForce(int);
void txFaultSpecific(int);
void txFaults();
void updateFaults();
void killFaultLibrary();
void handleCallbacks(fault_status_t);
bool currMCULatched();
bool warningLatched();
bool errorLatched();
bool fatalLatched();
bool otherMCUsLatched();
bool isLatched();
fault_attributes_t getFault(int id);

#endif