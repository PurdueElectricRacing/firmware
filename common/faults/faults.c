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
// #include "source/l4_testing/can/can_parse.h"



//BEGIN AUTO INCLUDES
#if FAULT_NODE_NAME == 0
	#include "source/main_module/can/can_parse.h"
#endif
#if FAULT_NODE_NAME == 1
	#include "source/driveline/can/can_parse.h"
#endif
#if FAULT_NODE_NAME == 2
	#include "source/dashboard/can/can_parse.h"
#endif
#if FAULT_NODE_NAME == 3
	#include "source/precharge/can/can_parse.h"
#endif
#if FAULT_NODE_NAME == 4
	#include "source/torque_vector/can/can_parse.h"
#endif
#if FAULT_NODE_NAME == 5
	#include "source/l4_testing/can/can_parse.h"
#endif
//END AUTO INCLUDES

//BEGIN AUTO FAULT INFO ARRAY DEFS
int idArray[TOTAL_NUM_FAULTS] = { ID_BATT_FLOW_FAULT, ID_DRIVE_FLOW_FAULT, ID_MAIN_COMM_FAULT, ID_LV_DEAD_FAULT, ID_MOT_FRONT_OT_FAULT, ID_WLSPD_L_FAULT,
			ID_WLSPD_R_FAULT, ID_DRIVELINE_COMM_FAULT, ID_BMS_FAULT, ID_BATT_OT_FAULT, ID_TV_OFFLINE_FAULT, ID_TEST_FAULT_1_FAULT, ID_TEST_FAULT_2_FAULT,
			ID_TEST_FAULT_3_FAULT, ID_TEST_FAULT_4_FAULT,};
int maxArray[TOTAL_NUM_FAULTS] = { BATT_FLOW_MAX, DRIVE_FLOW_MAX, MAIN_COMM_MAX, LV_DEAD_MAX, MOT_FRONT_OT_MAX, WLSPD_L_MAX,
			WLSPD_R_MAX, DRIVELINE_COMM_MAX, BMS_MAX, BATT_OT_MAX, TV_OFFLINE_MAX, TEST_FAULT_1_MAX, TEST_FAULT_2_MAX,
			TEST_FAULT_3_MAX, TEST_FAULT_4_MAX,};
int minArray[TOTAL_NUM_FAULTS] = { BATT_FLOW_MIN, DRIVE_FLOW_MIN, MAIN_COMM_MIN, LV_DEAD_MIN, MOT_FRONT_OT_MIN, WLSPD_L_MIN,
			WLSPD_R_MIN, DRIVELINE_COMM_MIN, BMS_MIN, BATT_OT_MIN, TV_OFFLINE_MIN, TEST_FAULT_1_MIN, TEST_FAULT_2_MIN,
			TEST_FAULT_3_MIN, TEST_FAULT_4_MIN,};
fault_priority_t priorityArray[TOTAL_NUM_FAULTS] = { BATT_FLOW_PRIORITY, DRIVE_FLOW_PRIORITY, MAIN_COMM_PRIORITY, LV_DEAD_PRIORITY, MOT_FRONT_OT_PRIORITY, WLSPD_L_PRIORITY,
			WLSPD_R_PRIORITY, DRIVELINE_COMM_PRIORITY, BMS_PRIORITY, BATT_OT_PRIORITY, TV_OFFLINE_PRIORITY, TEST_FAULT_1_PRIORITY, TEST_FAULT_2_PRIORITY,
			TEST_FAULT_3_PRIORITY, TEST_FAULT_4_PRIORITY,};
char msgArray[TOTAL_NUM_FAULTS][MAX_MSG_SIZE] = { BATT_FLOW_MSG, DRIVE_FLOW_MSG, MAIN_COMM_MSG, LV_DEAD_MSG, MOT_FRONT_OT_MSG, WLSPD_L_MSG,
			WLSPD_R_MSG, DRIVELINE_COMM_MSG, BMS_MSG, BATT_OT_MSG, TV_OFFLINE_MSG, TEST_FAULT_1_MSG, TEST_FAULT_2_MSG,
			TEST_FAULT_3_MSG, TEST_FAULT_4_MSG,};
int faultLatchTime[TOTAL_NUM_FAULTS] = { BATT_FLOW_LATCH_TIME, DRIVE_FLOW_LATCH_TIME, MAIN_COMM_LATCH_TIME, LV_DEAD_LATCH_TIME, MOT_FRONT_OT_LATCH_TIME, WLSPD_L_LATCH_TIME,
			WLSPD_R_LATCH_TIME, DRIVELINE_COMM_LATCH_TIME, BMS_LATCH_TIME, BATT_OT_LATCH_TIME, TV_OFFLINE_LATCH_TIME, TEST_FAULT_1_LATCH_TIME, TEST_FAULT_2_LATCH_TIME,
			TEST_FAULT_3_LATCH_TIME, TEST_FAULT_4_LATCH_TIME,};
int faultULatchTime[TOTAL_NUM_FAULTS] = { BATT_FLOW_UNLATCH_TIME, DRIVE_FLOW_UNLATCH_TIME, MAIN_COMM_UNLATCH_TIME, LV_DEAD_UNLATCH_TIME, MOT_FRONT_OT_UNLATCH_TIME, WLSPD_L_UNLATCH_TIME,
			WLSPD_R_UNLATCH_TIME, DRIVELINE_COMM_UNLATCH_TIME, BMS_UNLATCH_TIME, BATT_OT_UNLATCH_TIME, TV_OFFLINE_UNLATCH_TIME, TEST_FAULT_1_UNLATCH_TIME, TEST_FAULT_2_UNLATCH_TIME,
			TEST_FAULT_3_UNLATCH_TIME, TEST_FAULT_4_UNLATCH_TIME,};
//END AUTO FAULT INFO ARRAY DEFS

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
    if (array->forceActive)
    {
        return messageArray[GET_IDX(id)].latched;
    }
    array->tempLatch = ((valueToCompare > array->f_max) || (valueToCompare <= array->f_min)) ? true : false;
    return faultArray[GET_IDX(id)].tempLatch; /*array->tempLatch*/
}

fault_attributes_t getFault(int id) {
    return faultArray[GET_IDX(id)];
}



void txFaults() {
        // fault_message_t *message = &messageArray[GET_IDX(curridx++)];
        fault_message_t *message = &messageArray[curridx++];
        //BEGIN AUTO TX COMMAND
			#if FAULT_NODE_NAME == 0
             	SEND_FAULT_SYNC_MAIN_MODULE(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 1
             	SEND_FAULT_SYNC_DRIVELINE(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 2
             	SEND_FAULT_SYNC_DASHBOARD(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 3
             	SEND_FAULT_SYNC_PRECHARGE(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 4
             	SEND_FAULT_SYNC_TORQUE_VECTOR(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 5
             	SEND_FAULT_SYNC_L4_TESTING(*q_tx, message->f_ID, message->latched);
             #endif
        //END AUTO TX COMMAND
     if ((curridx >= TOTAL_NUM_FAULTS) || (GET_OWNER(faultArray[curridx].f_ID) != currentMCU)) {
        curridx = ownedidx;
     }
}

void txFaultSpecific(int id) {
    fault_message_t *message = &messageArray[GET_IDX(id)];
//BEGIN AUTO TX COMMAND SPECIFIC
			#if FAULT_NODE_NAME == 0
             	SEND_FAULT_SYNC_MAIN_MODULE(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 1
             	SEND_FAULT_SYNC_DRIVELINE(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 2
             	SEND_FAULT_SYNC_DASHBOARD(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 3
             	SEND_FAULT_SYNC_PRECHARGE(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 4
             	SEND_FAULT_SYNC_TORQUE_VECTOR(*q_tx, message->f_ID, message->latched);
             #endif
			#if FAULT_NODE_NAME == 5
             	SEND_FAULT_SYNC_L4_TESTING(*q_tx, message->f_ID, message->latched);
             #endif
//END AUTO TX COMMAND SPECIFIC
}

// void fault_sync_driveline_CALLBACK(CanParsedData_t *msg_header_a) {
//     fault_message_t recievedMessage = {msg_header_a->fault_sync_driveline.latched, msg_header_a->fault_sync_driveline.idx};
//     fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
//     switch (recievedMessage.latched) {
//         case true:
//             if (!currMessage->latched) {
//                 currMessage->latched = recievedMessage.latched;
//                 switch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {
//                     case INFO:
//                         infoCount++;
//                         break;
//                     case WARNING:
//                         warnCount++;
//                         break;
//                     case CRITICAL:
//                         critCount++;
//                         break;
//                 }
//             }
//             break;
//         case false:
//             if (currMessage->latched) {
//                 currMessage->latched = recievedMessage.latched;
//                 switch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {
//                     case INFO:
//                         infoCount--;
//                         break;
//                     case WARNING:
//                         warnCount--;
//                         break;
//                     case CRITICAL:
//                         critCount--;
//                         break;
//                 }
//             }
//             break;
//     }
// }


void handleCallbacks(fault_message_t recievedMessage, fault_message_t *currMessage) {
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

//BEGIN AUTO RECIEVE FUNCTIONS
void fault_sync_main_module_CALLBACK(CanParsedData_t *msg_header_a) {
	fault_message_t recievedMessage = {msg_header_a->fault_sync_main_module.latched, msg_header_a->fault_sync_main_module.idx};
	fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
	handleCallbacks(recievedMessage, currMessage);
}
void fault_sync_driveline_CALLBACK(CanParsedData_t *msg_header_a) {
	fault_message_t recievedMessage = {msg_header_a->fault_sync_driveline.latched, msg_header_a->fault_sync_driveline.idx};
	fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
	handleCallbacks(recievedMessage, currMessage);
}
void fault_sync_dashboard_CALLBACK(CanParsedData_t *msg_header_a) {
	fault_message_t recievedMessage = {msg_header_a->fault_sync_dashboard.latched, msg_header_a->fault_sync_dashboard.idx};
	fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
	handleCallbacks(recievedMessage, currMessage);
}
void fault_sync_precharge_CALLBACK(CanParsedData_t *msg_header_a) {
	fault_message_t recievedMessage = {msg_header_a->fault_sync_precharge.latched, msg_header_a->fault_sync_precharge.idx};
	fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
	handleCallbacks(recievedMessage, currMessage);
}
void fault_sync_torque_vector_CALLBACK(CanParsedData_t *msg_header_a) {
	fault_message_t recievedMessage = {msg_header_a->fault_sync_torque_vector.latched, msg_header_a->fault_sync_torque_vector.idx};
	fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
	handleCallbacks(recievedMessage, currMessage);
}
void fault_sync_l4_testing_CALLBACK(CanParsedData_t *msg_header_a) {
	fault_message_t recievedMessage = {msg_header_a->fault_sync_l4_testing.latched, msg_header_a->fault_sync_l4_testing.idx};
	fault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];
	handleCallbacks(recievedMessage, currMessage);
}
//END AUTO RECIEVE FUNCTIONS

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

void unForce(int id) {
    faultArray[GET_IDX(id)].forceActive = false;
}

void forceFault(int id, bool state) {
    uint16_t idx = GET_IDX(id);
    faultArray[idx].tempLatch = state;
    faultArray[idx].forceActive = true;
    messageArray[idx].latched = state;
	txFaultSpecific(id);

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
        fault_attributes_t tempAttribute = {false, false, priorityArray[i], 0, 0, 0, idArray[i], maxArray[i], minArray[i],
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