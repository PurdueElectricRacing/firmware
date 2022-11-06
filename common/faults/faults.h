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

//TODO: Make defs actually auto
//Begin Total Defs
#define TOTAL_MCU_NUM 6
#define TOTAL_MAIN_MODULE_FAULTS 2
#define TOTAL_DRIVELINE_FRONT_FAULTS 1
#define TOTAL_DASHBOARD_FAULTS 1
#define TOTAL_PRECHARGE_FAULTS 2
#define TOTAL_TV_FAULTS 1
//End Total Defs

//TODO: Make defs actually auto
//TODO: Create Hex scheme for defs (use CAN as example)
//Begin auto ID defs
#define ID_BMS_FAULT 0x00000001
#define ID_BATT_OT_FAULT 0x00000002
#define ID_MOT_REAR_OT_FAULT 0x0000003
#define ID_MOT_FRONT_OT_FAULT 0x00000004
#define ID_BATT_FLOW_FAULT 0x00000005
#define ID_DRIVE_FLOW_FAULT 0x00000006
#define TV_FAIL_FAULT 0x00000007
//End auto ID defs

//INFO: Doesn't affect driving state (Car can still safely drive)
//ERROR: Car exits ready2drive, but LV + HV systems still active
//CRITICAL: The Car SDC is activated
//Begin auto priority defs
#define BMS_PRIORITY 2
#define BATT_OT_PRIORITY 1
#define MOT_REAR_OT_PRIORITY 1
#define MOT_FRONT_OT_PRIORITY 1
#define BATT_FLOW_FAULT 1
#define DRIVE_FLOW_FAULT 1
#define TV_FAIL_PRIORITY 0
//End auto priority defs

//Begin auto max/min defs
#define BMS_MAX 1
#define BMS_MIN 0
#define BATT_OT_MAX 60
#define BATT_OT_MIN 10
#define MOT_REAR_OT_MAX 100
#define MOT_REAR_OT_MIN 10
#define MOT_FRONT_OT_MAX 100
#define MOT_FRONT_OT_MIN 10
#define BATT_FLOW_MAX 6.3
#define BATT_FLOW_MIN 4.3
#define DRIVE_FLOW_MAX 4.5
#define DRIVE_FLOW_MIN 6.5
#define TV_FAIL_MAX 1
#define TV_FAIL_MIN 0
//End auto max/min defs

//Begin auto screenmsg defs
#define BMS_MSG "BMS fault"
#define BATT_OT_MSG "Battery overheating"
#define MOT_REAR_OT_MSG "Rear motors overheating"
#define MOT_FRONT_OT_MSG "Front motors overheating"
#define BATT_FLOW_MSG "Battery loop flow incorrect"
#define DRIVE_FLOW_MSG "Drivetrain loop flow incorrect"
#define TV_FAIL_MSG "TV offline"
//End auto screenmsg defs

//TODO: Make defs actually auto
//Begin auto ENUM Defs
typedef enum {
    INFO,
    ERROR,
    CRITICAL
} fault_priority_t;

typedef enum {
    MAIN_MODULE,
    DRIVELINE_FRONT,
    DRIVELINE_REAR,
    DASHBOARD,
    PRECHARGE,
    TV
} fault_owner_t;



//End auto ENUM Defs

//Designed these two parts; fault message sends fault values through CAN w/o unneeded extra load of the attributes; Kept synced thru fault ID
typedef struct {
    bool latched : 1;
    int f_ID;
} fault_message_t;
typedef struct {
    fault_priority_t priority;
    fault_owner_t owner;
    int f_ID;
    int f_max;
    int f_min;
    char* screen_MSG;
} fault_attributes_t;




//Function defs
extern bool linkCarStateSDC();
extern void initFaultLibrary();
extern bool setFault(fault_message_t*, int);
extern void killFaultLibrary();

#endif