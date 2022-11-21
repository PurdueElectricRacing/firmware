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

#include "source/driveline/can/can_parse.h"
// #include "source/l4_testing/can/can_parse.h"

//Begin fault info array defs
int idArray[TOTAL_NUM_FAULTS] = {ID_BMS_FAULT, ID_BATT_OT_FAULT, ID_MOT_REAR_OT_FAULT, ID_MOT_FRONT_OT_FAULT, ID_WLSPD_L_FAULT, ID_WLSPD_R_FAULT, ID_DRIVELINE_COMM_FAULT,
                                ID_BATT_FLOW_FAULT, ID_DRIVE_FLOW_FAULT, ID_MAIN_COMM_FAULT, ID_LV_DEAD_FAULT, TV_FAIL_FAULT};
int maxArray[TOTAL_NUM_FAULTS] = {BMS_MAX, BATT_OT_MAX, MOT_REAR_OT_MAX, MOT_FRONT_OT_MAX, WLSPD_L_MAX, WLSPD_R_MAX, DRIVELINE_COMM_MAX, BATT_FLOW_MAX,
                                DRIVE_FLOW_MAX, MAIN_COMM_MAX, LV_DEAD_MAX, TV_FAIL_MAX};
int minArray[TOTAL_NUM_FAULTS] = {BMS_MIN, BATT_OT_MIN, MOT_REAR_OT_MIN, MOT_FRONT_OT_MIN, WLSPD_L_MIN, WLSPD_R_MIN, DRIVELINE_COMM_MIN, BATT_FLOW_MIN,
                                    DRIVE_FLOW_MIN, MAIN_COMM_MIN, LV_DEAD_MIN, TV_FAIL_MIN};
fault_priority_t priorityArray[TOTAL_NUM_FAULTS] = {BMS_PRIORITY, BATT_OT_PRIORITY, MOT_REAR_OT_PRIORITY, MOT_FRONT_OT_PRIORITY, WLSPD_L_PRIORITY, WLSPD_R_PRIORITY, DRIVELINE_COMM_PRIORITY,
                                                    BATT_FLOW_PRIORITY, DRIVE_FLOW_PRIORITY, MAIN_COMM_PRIORITY, LV_DEAD_PRIORITY, TV_FAIL_PRIORITY};
char msgArray[TOTAL_NUM_FAULTS][MAX_MSG_SIZE] = {BMS_MSG, BATT_OT_MSG, MOT_REAR_OT_MSG, MOT_FRONT_OT_MSG, WLSPD_L_MSG, WLSPD_R_MSG, DRIVELINE_COMM_MSG,
                                                BATT_FLOW_MSG, DRIVE_FLOW_MSG, MAIN_COMM_MSG, LV_DEAD_MSG, TV_FAIL_MSG};
int faultLatchTime[TOTAL_NUM_FAULTS] = {BMS_L_TIME, BATT_OT_L_TIME, MOT_REAR_OT_L_TIME, MOT_FRONT_OT_L_TIME, WLSPD_L_L_TIME, WLSPD_R_L_TIME, DRIVELINE_COMM_L_TIME,
                                        BATT_FLOW_L_TIME, DRIVE_FLOW_L_TIME, MAIN_COMM_L_TIME, LV_DEAD_L_TIME, TV_FAIL_L_TIME};
int faultULatchTime[TOTAL_NUM_FAULTS] = {BMS_UL_TIME, BATT_OT_UL_TIME, MOT_REAR_OT_UL_TIME, MOT_FRONT_OT_UL_TIME, WLSPD_L_UL_TIME, WLSPD_R_UL_TIME, DRIVELINE_COMM_UL_TIME,
                                        BATT_FLOW_UL_TIME, DRIVE_FLOW_UL_TIME, MAIN_COMM_UL_TIME, LV_DEAD_UL_TIME, TV_FAIL_UL_TIME};
//End fault info array defs

fault_owner_t currentMCU;

fault_attributes_t faultArray[TOTAL_NUM_FAULTS];
fault_message_t messageArray[TOTAL_NUM_FAULTS];

uint16_t critCount;
uint16_t warnCount;
uint16_t infoCount;
uint16_t currCount;

q_handle_t *q_tx;
q_handle_t *q_rx;

uint16_t ownedidx;
uint16_t curridx;


bool fault_lib_enable;
//MAX exclusive, MIN inclusive
bool setFault(int id, int valueToCompare) {
    if (!fault_lib_enable)
    {
        return false;
    }
    if (GET_OWNER(id) != currentMCU) {
        return false;
    }

    fault_attributes_t *array = &faultArray[GET_IDX(id)];
    array->tempLatch = ((valueToCompare > array->f_max) || (valueToCompare <= array->f_min)) ? true : false;
    return faultArray[GET_IDX(id)].tempLatch; /*array->tempLatch*/
}

fault_attributes_t getFault(int id) {
    return faultArray[GET_IDX(id)];
}



void txFaults() {
        fault_message_t *message = &messageArray[GET_IDX(curridx++)];
        //Begin auto send command
        switch(currentMCU) {
            case MAIN_MODULE:
                // SEND_FAULT_SYNC_TEST(*q_tx, message->f_ID, message->latched);
                break;
            case DRIVELINE_FRONT:
                SEND_FAULT_SYNC_DRIVELINE(*q_tx, message->f_ID, message->latched);
                break;

        }
        //End auto send command
     if ((curridx >= TOTAL_NUM_FAULTS) || (GET_OWNER(faultArray[curridx].f_ID) != currentMCU)) {
        curridx = ownedidx;
     }
}

void txFaultSpecific(int id) {
    fault_message_t *message = &messageArray[GET_IDX(id)];
        switch(currentMCU) {
            case MAIN_MODULE:
                // SEND_FAULT_SYNC_TEST(*q_tx, message->f_ID, message->latched);
                break;
            case DRIVELINE_FRONT:
                SEND_FAULT_SYNC_DRIVELINE(*q_tx, message->f_ID, message->latched);
                break;

        }
}

//Begin auto rx functions
void fault_sync_driveline_CALLBACK(CanParsedData_t *msg_header_a) {
    fault_message_t recievedMessage = {msg_header_a->fault_sync_driveline.latched, msg_header_a->fault_sync_driveline.idx};
    fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
    switch (recievedMessage.latched) {
        case true:
            if (!currMessage->latched) {
                currMessage->latched = recievedMessage.latched;
                switch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {
                    case INFO:
                        infoCount++;
                        break;
                    case WARNING:
                        warnCount++;
                        break;
                    case CRITICAL:
                        critCount++;
                        break;
                }
            }
            break;
        case false:
            if (currMessage->latched) {
                currMessage->latched = recievedMessage.latched;
                switch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {
                    case INFO:
                        infoCount--;
                        break;
                    case WARNING:
                        warnCount--;
                        break;
                    case CRITICAL:
                        critCount--;
                        break;
                }
            }
            break;
    }
}

void fault_sync_test_CALLBACK(CanParsedData_t *msg_header_a) {
    fault_message_t recievedMessage = {msg_header_a->fault_sync_test.latched, msg_header_a->fault_sync_test.idx};
    fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
    switch (recievedMessage.latched) {
        case true:
            if (!currMessage->latched) {
                currMessage->latched = recievedMessage.latched;
                switch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {
                    case INFO:
                        infoCount++;
                        break;
                    case WARNING:
                        warnCount++;
                        break;
                    case CRITICAL:
                        critCount++;
                        break;
                }
            }
            break;
        case false:
            if (currMessage->latched) {
                currMessage->latched = recievedMessage.latched;
                switch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {
                    case INFO:
                        infoCount--;
                        break;
                    case WARNING:
                        warnCount--;
                        break;
                    case CRITICAL:
                        critCount--;
                        break;
                }
            }
            break;
    }
}
//End auto rx functions

void updateFaults() {
    uint16_t idx = ownedidx;
    fault_attributes_t *fault;
    do {
        fault = &faultArray[idx];
        if (((fault->tempLatch) && !(fault->message->latched))) {
            fault->message->latched = (fault->time_since_latch++ >= faultLatchTime[idx]) ? true : false;
            if (fault->message->latched) {
                currCount++;
                switch(fault->priority) {
                    case INFO:
                        infoCount++;
                        break;
                    case WARNING:
                        warnCount++;
                        break;
                    case CRITICAL:
                        critCount++;
                        break;
                }
                txFaultSpecific(idx);
            }
        }
        else if (!(fault->tempLatch) && (fault->message->latched)) {
            fault->message->latched = (fault->time_since_latch++ >= faultULatchTime[idx]) ? false : true;
            if (!(fault->message->latched)) {
                currCount--;
                switch(fault->priority) {
                    case INFO:
                        infoCount--;
                        break;
                    case WARNING:
                        warnCount--;
                        break;
                    case CRITICAL:
                        critCount--;
                        break;
                }
                txFaultSpecific(idx);
            }
        }
        else {
            fault->time_since_latch = 0;
        }
        idx++;
    } while ((idx < TOTAL_NUM_FAULTS) && (GET_OWNER(faultArray[idx].f_ID) == currentMCU));
}

void killFaultLibrary() {
    fault_lib_enable = false;
}

bool currMCULatched() {
    return (currCount == 0) ? false : true;
}

bool infoLatched() {
    return (infoCount == 0) ? false : true;
}

bool warningLatched() {
    return (warnCount == 0) ? false : true;
}

bool criticalLatched() {
    return (critCount == 0) ? false : true;
}

bool isLatched() {
    return (infoCount + warnCount + critCount == 0) ? false : true;
}

bool checkFault(int id) {
    return message[GET_IDX(id)].latched;
}

void forceFault(int id, bool state) {
    faultArray[GET_IDX(id)].tempLatch = state;
}

void initFaultLibrary(fault_owner_t mcu, q_handle_t* txQ, q_handle_t* rxQ) {
    fault_lib_enable = true;
    bool foundStartIdx = false;
    q_tx = txQ;
    q_rx = rxQ;
    currentMCU = mcu;
    uint16_t num_owned_faults = 0;
    uint16_t num_recieved_faults = 0;
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
        fault_message_t tempMsg = {false, idArray[i]};
        messageArray[i] = tempMsg;
        fault_attributes_t tempAttribute = {0, priorityArray[i], 0, 0, idArray[i], maxArray[i], minArray[i],
                        &messageArray[i], msgArray[i]};
        faultArray[i] = tempAttribute;
        if (GET_OWNER(idArray[i]) == mcu && !foundStartIdx) {
            foundStartIdx = true;
            ownedidx = i;
        }
    }
    curridx = ownedidx;
    warnCount = 0;
    infoCount = 0;
    critCount = 0;
    currCount = 0;
}