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
#include "common/phal_L4/can/can.h"

//BEGIN AUTO FAULT INFO ARRAY DEFS
uint16_t faultLatchTime[TOTAL_NUM_FAULTS] = { BATT_FLOW_LATCH_TIME, DRIVE_FLOW_LATCH_TIME, MAIN_COMM_LATCH_TIME, LV_DEAD_LATCH_TIME, MOT_FRONT_OT_LATCH_TIME, WLSPD_L_LATCH_TIME,
			WLSPD_R_LATCH_TIME, DRIVELINE_COMM_LATCH_TIME, APPS_WIRING_T1_LATCH_TIME, APPS_WIRING_T2_LATCH_TIME, BSE_WIRING_B1_LATCH_TIME, BSE_WIRING_B2_LATCH_TIME, BSE_WIRING_B3_LATCH_TIME,
			IMPLAUS_DETECTED_LATCH_TIME, APPS_BRAKE_LATCH_TIME, BATT_OT_LATCH_TIME, TV_OFFLINE_LATCH_TIME, TEST_FAULT_1_LATCH_TIME, TEST_FAULT_2_LATCH_TIME, TEST_FAULT_3_LATCH_TIME,
			TEST_FAULT_4_LATCH_TIME,};
uint16_t faultULatchTime[TOTAL_NUM_FAULTS] = { BATT_FLOW_UNLATCH_TIME, DRIVE_FLOW_UNLATCH_TIME, MAIN_COMM_UNLATCH_TIME, LV_DEAD_UNLATCH_TIME, MOT_FRONT_OT_UNLATCH_TIME, WLSPD_L_UNLATCH_TIME,
			WLSPD_R_UNLATCH_TIME, DRIVELINE_COMM_UNLATCH_TIME, APPS_WIRING_T1_UNLATCH_TIME, APPS_WIRING_T2_UNLATCH_TIME, BSE_WIRING_B1_UNLATCH_TIME, BSE_WIRING_B2_UNLATCH_TIME, BSE_WIRING_B3_UNLATCH_TIME,
			IMPLAUS_DETECTED_UNLATCH_TIME, APPS_BRAKE_UNLATCH_TIME, BATT_OT_UNLATCH_TIME, TV_OFFLINE_UNLATCH_TIME, TEST_FAULT_1_UNLATCH_TIME, TEST_FAULT_2_UNLATCH_TIME, TEST_FAULT_3_UNLATCH_TIME,
			TEST_FAULT_4_UNLATCH_TIME,};
//Global arrays with all faults
fault_status_t statusArray[TOTAL_NUM_FAULTS] = {
	(fault_status_t){false, ID_BATT_FLOW_FAULT},
	(fault_status_t){false, ID_DRIVE_FLOW_FAULT},
	(fault_status_t){false, ID_MAIN_COMM_FAULT},
	(fault_status_t){false, ID_LV_DEAD_FAULT},
	(fault_status_t){false, ID_MOT_FRONT_OT_FAULT},
	(fault_status_t){false, ID_WLSPD_L_FAULT},
	(fault_status_t){false, ID_WLSPD_R_FAULT},
	(fault_status_t){false, ID_DRIVELINE_COMM_FAULT},
	(fault_status_t){false, ID_APPS_WIRING_T1_FAULT},
	(fault_status_t){false, ID_APPS_WIRING_T2_FAULT},
	(fault_status_t){false, ID_BSE_WIRING_B1_FAULT},
	(fault_status_t){false, ID_BSE_WIRING_B2_FAULT},
	(fault_status_t){false, ID_BSE_WIRING_B3_FAULT},
	(fault_status_t){false, ID_IMPLAUS_DETECTED_FAULT},
	(fault_status_t){false, ID_APPS_BRAKE_FAULT},
	(fault_status_t){false, ID_BATT_OT_FAULT},
	(fault_status_t){false, ID_TV_OFFLINE_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_1_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_2_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_3_FAULT},
	(fault_status_t){false, ID_TEST_FAULT_4_FAULT},
};
fault_attributes_t faultArray[TOTAL_NUM_FAULTS] = {
	(fault_attributes_t){false, false, BATT_FLOW_PRIORITY, 0, 0, BATT_FLOW_MAX, BATT_FLOW_MIN, &statusArray[0], BATT_FLOW_MSG},
	(fault_attributes_t){false, false, DRIVE_FLOW_PRIORITY, 0, 0, DRIVE_FLOW_MAX, DRIVE_FLOW_MIN, &statusArray[1], DRIVE_FLOW_MSG},
	(fault_attributes_t){false, false, MAIN_COMM_PRIORITY, 0, 0, MAIN_COMM_MAX, MAIN_COMM_MIN, &statusArray[2], MAIN_COMM_MSG},
	(fault_attributes_t){false, false, LV_DEAD_PRIORITY, 0, 0, LV_DEAD_MAX, LV_DEAD_MIN, &statusArray[3], LV_DEAD_MSG},
	(fault_attributes_t){false, false, MOT_FRONT_OT_PRIORITY, 0, 0, MOT_FRONT_OT_MAX, MOT_FRONT_OT_MIN, &statusArray[4], MOT_FRONT_OT_MSG},
	(fault_attributes_t){false, false, WLSPD_L_PRIORITY, 0, 0, WLSPD_L_MAX, WLSPD_L_MIN, &statusArray[5], WLSPD_L_MSG},
	(fault_attributes_t){false, false, WLSPD_R_PRIORITY, 0, 0, WLSPD_R_MAX, WLSPD_R_MIN, &statusArray[6], WLSPD_R_MSG},
	(fault_attributes_t){false, false, DRIVELINE_COMM_PRIORITY, 0, 0, DRIVELINE_COMM_MAX, DRIVELINE_COMM_MIN, &statusArray[7], DRIVELINE_COMM_MSG},
	(fault_attributes_t){false, false, APPS_WIRING_T1_PRIORITY, 0, 0, APPS_WIRING_T1_MAX, APPS_WIRING_T1_MIN, &statusArray[8], APPS_WIRING_T1_MSG},
	(fault_attributes_t){false, false, APPS_WIRING_T2_PRIORITY, 0, 0, APPS_WIRING_T2_MAX, APPS_WIRING_T2_MIN, &statusArray[9], APPS_WIRING_T2_MSG},
	(fault_attributes_t){false, false, BSE_WIRING_B1_PRIORITY, 0, 0, BSE_WIRING_B1_MAX, BSE_WIRING_B1_MIN, &statusArray[10], BSE_WIRING_B1_MSG},
	(fault_attributes_t){false, false, BSE_WIRING_B2_PRIORITY, 0, 0, BSE_WIRING_B2_MAX, BSE_WIRING_B2_MIN, &statusArray[11], BSE_WIRING_B2_MSG},
	(fault_attributes_t){false, false, BSE_WIRING_B3_PRIORITY, 0, 0, BSE_WIRING_B3_MAX, BSE_WIRING_B3_MIN, &statusArray[12], BSE_WIRING_B3_MSG},
	(fault_attributes_t){false, false, IMPLAUS_DETECTED_PRIORITY, 0, 0, IMPLAUS_DETECTED_MAX, IMPLAUS_DETECTED_MIN, &statusArray[13], IMPLAUS_DETECTED_MSG},
	(fault_attributes_t){false, false, APPS_BRAKE_PRIORITY, 0, 0, APPS_BRAKE_MAX, APPS_BRAKE_MIN, &statusArray[14], APPS_BRAKE_MSG},
	(fault_attributes_t){false, false, BATT_OT_PRIORITY, 0, 0, BATT_OT_MAX, BATT_OT_MIN, &statusArray[15], BATT_OT_MSG},
	(fault_attributes_t){false, false, TV_OFFLINE_PRIORITY, 0, 0, TV_OFFLINE_MAX, TV_OFFLINE_MIN, &statusArray[16], TV_OFFLINE_MSG},
	(fault_attributes_t){false, false, TEST_FAULT_1_PRIORITY, 0, 0, TEST_FAULT_1_MAX, TEST_FAULT_1_MIN, &statusArray[17], TEST_FAULT_1_MSG},
	(fault_attributes_t){false, false, TEST_FAULT_2_PRIORITY, 0, 0, TEST_FAULT_2_MAX, TEST_FAULT_2_MIN, &statusArray[18], TEST_FAULT_2_MSG},
	(fault_attributes_t){false, false, TEST_FAULT_3_PRIORITY, 0, 0, TEST_FAULT_3_MAX, TEST_FAULT_3_MIN, &statusArray[19], TEST_FAULT_3_MSG},
	(fault_attributes_t){false, false, TEST_FAULT_4_PRIORITY, 0, 0, TEST_FAULT_4_MAX, TEST_FAULT_4_MIN, &statusArray[20], TEST_FAULT_4_MSG},
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

    fault_attributes_t *fault = &faultArray[GET_IDX(id)];

    //The fault is being forced to be a certain value, stop running
    if (fault->forceActive)
    {
        return statusArray[GET_IDX(id)].latched;
    }
    //Templatch = result of comparison of limits + current value
    fault->tempLatch = ((valueToCompare >= fault->f_max) || (valueToCompare < fault->f_min)) ? true : false;
    return faultArray[GET_IDX(id)].tempLatch;
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
void updateFaults() {
    if (ownedidx < 0 || fault_lib_disable) {
        return;
    }
    uint16_t idx = ownedidx;
    fault_attributes_t *fault;
    do {
        fault = &faultArray[idx];
        //Fault is showing up as latched
        if (((fault->tempLatch) && !(fault->status->latched))) {
            //Has latching period ended
            fault->status->latched = (++fault->time_since_latch >= faultLatchTime[idx]) ? true : false;
            if (fault->status->latched) {
                fault->time_since_latch = 0;
                currCount++;
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
            //Make sure unlatch period has elapsed
            fault->status->latched = (++fault->time_since_latch >= faultULatchTime[idx]) ? false : true;
            if (!(fault->status->latched)) {
                fault->time_since_latch = 0;
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
            //Account for potential noise during the latching process
            if (fault->time_since_latch > 0 && fault->bounces <= (uint16_t)(faultLatchTime[idx] * 0.4)&& fault->tempLatch == 1) {
                fault->time_since_latch++;
                fault->bounces++;
            }
            else if (fault->time_since_latch > 0 && fault->bounces <= (uint16_t)(faultLatchTime[idx] * 0.4) && fault->tempLatch == 0) {
                fault->time_since_latch++;
                fault->bounces++;
            }
            else if (fault->time_since_latch > 0 && fault->bounces > (uint16_t)(faultLatchTime[idx] * 0.4) && fault->tempLatch == 1) {
                fault->time_since_latch = 0;
                fault->status->latched = 1;
                fault->tempLatch = 1;
                currCount++;
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
            }
            else if (fault->time_since_latch > 0 && fault->bounces > (uint16_t)(faultULatchTime[idx] * 0.4) && fault->tempLatch == 0) {
                fault->time_since_latch = 0;
                fault->status->latched = 1;
                fault->tempLatch = 1;
                currCount++;
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
            }
            else {
                fault->time_since_latch = 0;
            }
        }
        idx++;
    } while ((idx < TOTAL_NUM_FAULTS) && (GET_OWNER(faultArray[idx].status->f_ID) == currentMCU));
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
    return message[GET_IDX(id)].latched;
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
}