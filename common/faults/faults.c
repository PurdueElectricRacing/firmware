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

// #define ID_BMS_FAULT 0x00000001
// #define ID_BATT_OT_FAULT 0x00000002
// #define ID_MOT_OT_FAULT 0x0000003
// #define ID_BATT_FLOW_FAULT 0x00000004
// #define ID_DRIVE_FLOW_FAULT 0x00000005

fault_owner_t owner;

//Begin auto array declarations
// fault_message_t fault_status[TOTAL]
//End auto array declarations

//Begin auto fault message struct defs
// fault_message_t bms_fault = {false}
//End auto fault message struct defs
bool fault_lib_enable;

bool setFault(fault_message_t *fault_message, int valueToCompare) {
    if (!fault_lib_enable)
    {
        return false;
    }

}

void killFaultLibrary() {
    fault_lib_enable = false;
}

void initFaultLibrary(fault_owner_t mcu) {
    fault_lib_enable = true;
    owner = mcu;
}