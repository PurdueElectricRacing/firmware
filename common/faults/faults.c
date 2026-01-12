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
#include "common/can_library/generated/fault_data.h"
#if defined(STM32F407xx)
#include "common/phal_F4_F7/can/can.h"
#elif defined(STM32F732xx)
#include "common/phal_F4_F7/can/can.h"
#else
#include "common/phal_L4/can/can.h"
#endif
#include "common/psched/psched.h"

// The following arrays are now defined in common/can_library/generated/fault_data.c
// which should be included in the build for each node.

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
    if (fault_lib_disable || GET_OWNER(id) != currentMCU) {
        return false;
    }
    uint16_t idx              = GET_IDX(id);
    fault_attributes_t *fault = &faultArray[idx];

    //The fault is being forced to be a certain value, stop running
    if (fault->forceActive) {
        return statusArray[idx].latched;
    }
    //Templatch = result of comparison of limits + current value
    fault->tempLatch =
        ((valueToCompare >= fault->f_max) || (valueToCompare < fault->f_min)) ? true : false;
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
    fault_status_t *status     = &statusArray[curridx++];
    CanMsgTypeDef_t msg        = {.Bus = CAN1, .ExtId = can_ext, .DLC = 3, .IDE = 1};
    fault_can_format_t *data_a = (fault_can_format_t *)&msg.Data;
    data_a->fault_sync.idx     = status->f_ID;
    data_a->fault_sync.latched = status->latched;
    qSendToBack(q_tx, &msg);
    //Move to the next fault in the owned array
    if ((curridx >= TOTAL_NUM_FAULTS)
        || (GET_OWNER(faultArray[curridx].status->f_ID) != currentMCU)) {
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
    fault_status_t *status     = &statusArray[GET_IDX(id)];
    CanMsgTypeDef_t msg        = {.Bus = CAN1, .ExtId = can_ext, .DLC = 3, .IDE = 1};
    fault_can_format_t *data_a = (fault_can_format_t *)&msg.Data;
    data_a->fault_sync.idx     = status->f_ID;
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
    fault_status_t recievedStatus = (fault_status_t) {latched, id};
    fault_status_t *currStatus    = &statusArray[GET_IDX(recievedStatus.f_ID)];
    if (recievedStatus.latched) {
        //If current Message = 0, and recieved message = 1 (fault is latching)
        if (!currStatus->latched) {
            currStatus->latched = recievedStatus.latched;
            most_recent_latched = GET_IDX(id);
            qSendToBack(&q_fault_history, &most_recent_latched);
            switch (faultArray[GET_IDX(recievedStatus.f_ID)].priority) {
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
            switch (faultArray[GET_IDX(recievedStatus.f_ID)].priority) {
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
    uint32_t curr_time        = sched.os_ticks;
    //Fault is showing up as latched
    if (((fault->tempLatch) && !(fault->status->latched))) {
        fault->time_since_latch = curr_time - fault->start_ticks;
        //Has latching period ended
        fault->status->latched = (fault->time_since_latch >= faultLatchTime[idx]) ? true : false;
        if (fault->status->latched) {
            fault->time_since_latch = 0;
            fault->start_ticks      = curr_time;
            fault->bounces          = 0;
            currCount++;
            most_recent_latched = idx;
            qSendToBack(&q_fault_history, &idx);
            switch (fault->priority) {
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
            fault->start_ticks      = curr_time;
            fault->bounces          = 0;
            currCount--;
            switch (fault->priority) {
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
    } else {
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
        fault->start_ticks      = curr_time;
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
        switch (faultArray[idx].priority) {
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
        switch (faultArray[idx].priority) {
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
    faultArray[idx].tempLatch        = state;
    faultArray[idx].forceActive      = true;
    faultArray[idx].time_since_latch = 0;
    faultArray[idx].bounces          = 0;
    faultArray[idx].start_ticks      = sched.os_ticks;
    statusArray[idx].latched         = state;
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
void initFaultLibrary(uint8_t mcu, q_handle_t *txQ, uint32_t ext) {
    most_recent_latched          = 0xFFFF;
    fault_lib_disable            = false;
    bool foundStartIdx           = false;
    can_ext                      = ext;
    q_tx                         = txQ;
    currentMCU                   = mcu;
    uint16_t num_owned_faults    = 0;
    uint16_t num_recieved_faults = 0;
    //Find the beginning of current mcu's faults, if a fault exists
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
        if (GET_OWNER(faultArray[i].status->f_ID) == mcu && !foundStartIdx) {
            foundStartIdx = true;
            ownedidx      = i;
        }
    }
    if (!foundStartIdx) {
        ownedidx = -1;
    }
    curridx    = ownedidx;
    warnCount  = 0;
    errorCount = 0;
    fatalCount = 0;
    currCount  = 0;
    qConstruct(&q_fault_history, sizeof(uint16_t));
}