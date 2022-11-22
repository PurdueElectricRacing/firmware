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

// #include "common/phal_L4/can/can.h"
#include "common/queue/queue.h"


//TODO: Make defs actually aut
//BEGIN AUTO TOTAL DEFS
#define TOTAL_MCU_NUM 6
#define TOTAL_NUM_FAULTS 12
#define TOTAL_MAIN_MODULE_FAULTS 2
#define TOTAL_DRIVELINE_FRONT_FAULTS 3
#define TOTAL_DRIVELINE_REAR_FAULTS 1
#define TOTAL_DASHBOARD_FAULTS 0
#define TOTAL_PRECHARGE_FAULTS 2
#define TOTAL_TV_FAULTS 1
#define MAX_MSG_SIZE 100
//END AUTO TOTAL DEFS

//TODO: Make defs actually auto
//TODO: Create Hex scheme for defs (use CAN as example)
//BEGIN AUTO ID DEFS
#define ID_BMS_FAULT 0x3000
#define ID_BATT_OT_FAULT 0x4001
#define ID_MOT_REAR_OT_FAULT 0x1002
#define ID_MOT_FRONT_OT_FAULT 0x2003
#define ID_WLSPD_L_FAULT 0x2004
#define ID_WLSPD_R_FAULT 0x2005
#define ID_DRIVELINE_COMM_FAULT 0x2006
#define ID_BATT_FLOW_FAULT 0x0007
#define ID_DRIVE_FLOW_FAULT 0x0008
#define ID_MAIN_COMM_FAULT 0x0009
#define ID_LV_DEAD_FAULT 0x000a
#define TV_FAIL_FAULT 0x500b
//END AUTO ID DEFS
#define GET_IDX(id) (id & 0x0FFF)
#define GET_OWNER(id) ((id & 0xF000) >> 12)

//INFO: Doesn't affect driving state (Car can still safely drive)
//ERROR: Car exits ready2drive, but LV + HV systems still active
//CRITICAL: The Car SDC is activated
//BEGIN AUTO PRIORITY DEFS
#define BMS_PRIORITY 2
#define BATT_OT_PRIORITY 1
#define MOT_REAR_OT_PRIORITY 1
#define MOT_FRONT_OT_PRIORITY 1
#define WLSPD_L_PRIORITY 0
#define WLSPD_R_PRIORITY 0
#define DRIVELINE_COMM_PRIORITY 1
#define BATT_FLOW_PRIORITY 1
#define DRIVE_FLOW_PRIORITY 1
#define MAIN_COMM_PRIORITY 2
#define LV_DEAD_PRIORITY 2
#define TV_FAIL_PRIORITY 0
//END AUTO PRIORITY DEFS

//BEGIN AUTO MAX DEFS
//END AUTO MAX DEFS

//BEGIN AUTO MIN DEFS
//END AUTO MIN DEFS

//Begin auto max/min defs
#define BMS_MAX 1
#define BMS_MIN 0
#define BATT_OT_MAX 60
#define BATT_OT_MIN 10
#define MOT_REAR_OT_MAX 100
#define MOT_REAR_OT_MIN 10
#define MOT_FRONT_OT_MAX 100
#define MOT_FRONT_OT_MIN 10
#define WLSPD_L_MAX 4096
#define WLSPD_L_MIN 0
#define WLSPD_R_MAX 4096
#define WLSPD_R_MIN 0
#define DRIVELINE_COMM_MAX 1
#define DRIVELINE_COMM_MIN 0
#define BATT_FLOW_MAX 63
#define BATT_FLOW_MIN 43
#define DRIVE_FLOW_MAX 45
#define DRIVE_FLOW_MIN 65
#define MAIN_COMM_MAX 1
#define MAIN_COMM_MIN 0
#define LV_DEAD_MAX 315
#define LV_DEAD_MIN 200
#define TV_FAIL_MAX 1
#define TV_FAIL_MIN 0
//End auto max/min defs

//BEGIN AUTO LATCH DEFS
//END AUTO LATCH DEFS

//BEGIN AUTO UNLATCH DEFS
//END AUTO UNLATCH DEFS

//Begin auto l/ul defs
#define BMS_L_TIME 10
#define BMS_UL_TIME 10
#define BATT_OT_L_TIME 10
#define BATT_OT_UL_TIME 10
#define MOT_REAR_OT_L_TIME 10
#define MOT_REAR_OT_UL_TIME 10
#define MOT_FRONT_OT_L_TIME 10
#define MOT_FRONT_OT_UL_TIME 10
#define WLSPD_L_L_TIME 10
#define WLSPD_L_UL_TIME 10
#define WLSPD_R_L_TIME 10
#define WLSPD_R_UL_TIME 10
#define DRIVELINE_COMM_L_TIME 10
#define DRIVELINE_COMM_UL_TIME 10
#define BATT_FLOW_L_TIME 10
#define BATT_FLOW_UL_TIME 10
#define DRIVE_FLOW_L_TIME 10
#define DRIVE_FLOW_UL_TIME 10
#define MAIN_COMM_L_TIME 10
#define MAIN_COMM_UL_TIME 10
#define LV_DEAD_L_TIME 10
#define LV_DEAD_UL_TIME 10
#define TV_FAIL_L_TIME 10
#define TV_FAIL_UL_TIME 10
//End auto l/ul defs

//BEGIN AUTO SCREENMSG DEFS
#define BMS_MSG "BMS fault\0"
#define BATT_OT_MSG "Battery overheating\0"
#define MOT_REAR_OT_MSG "Rear motors overheating\0"
#define MOT_FRONT_OT_MSG "Front motors overheating\0"
#define WLSPD_L_MSG "Left Wheelspeed failed\0"
#define WLSPD_R_MSG "Right wheelspeed failed\0"
#define DRIVELINE_COMM_MSG "Driveline not responding\0"
#define BATT_FLOW_MSG "Battery loop flow incorrect\0"
#define DRIVE_FLOW_MSG "Drivetrain loop flow incorrect\0"
#define MAIN_COMM_MSG "Main Module not responding\0"
#define LV_DEAD_MSG "LV voltage too low, turn off ASAP\0"
#define TV_FAIL_MSG "TV offline\0"
//END AUTO SCREENMSG DEFS


//TODO: Make defs actually auto

typedef enum {
    INFO = 0,
    WARNING = 1,
    CRITICAL = 2
} fault_priority_t;
//BEGIN AUTO ENUM DEFS
typedef enum {
    MAIN_MODULE = 0,
    DRIVELINE_REAR = 1,
    DRIVELINE_FRONT = 2,
    DASHBOARD = 3,
    PRECHARGE = 4,
    TV = 5
} fault_owner_t;
//END AUTO ENUM DEFS



//Designed these two parts; fault message sends fault values through CAN w/o unneeded extra load of the attributes; Kept synced thru fault ID
typedef struct {
    bool latched;
    int f_ID;
} fault_message_t;
typedef struct {
    bool tempLatch;
    fault_priority_t priority;
    uint16_t time_since_latch;
    uint16_t last_rx_time;
    int f_ID;
    int f_max;
    int f_min;
    fault_message_t *message;
    char* screen_MSG;
} fault_attributes_t;




//Vars
extern fault_message_t message[TOTAL_NUM_FAULTS];
extern fault_attributes_t attributes[TOTAL_NUM_FAULTS];

//Function defs
extern void initFaultLibrary(fault_owner_t, q_handle_t*, q_handle_t*);
extern bool setFault(int, int);
extern void forceFault(int, bool);
extern void txFaultSpecific(int);
extern void txFaults();
extern void rxFaults();
extern void updateFaults();
extern void recieveFaultsPeriodic();
extern void killFaultLibrary();
extern bool currMCULatched();
extern bool infoLatched();
extern bool warningLatched();
extern bool criticalLatched();
extern bool isLatched();
extern bool checkFault();
extern fault_attributes_t getFault(int id);

#endif