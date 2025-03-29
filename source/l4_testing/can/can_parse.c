/**
 * @file can_parse.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief Parsing of CAN messages using auto-generated structures with bit-fields
 * @version 0.1
 * @date 2021-09-15
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "can_parse.h"

// prototypes
bool initCANFilter();

can_data_t can_data;
q_handle_t* q_rx_can_a;

void initCANParse(q_handle_t* rx_a)
{
    q_rx_can_a = rx_a;
    initCANFilter();
}

void canRxUpdate(void)
{
    CanMsgTypeDef_t msg_header;
    CanParsedData_t* msg_data_a;

    if(qReceive(q_rx_can_a, &msg_header) == SUCCESS_G)
    {
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        /* BEGIN AUTO CASES */
        switch(msg_header.ExtId)
        {
            case ID_TEST_MSG5_2:
                can_data.test_msg5_2.test_sig5 = msg_data_a->test_msg5_2.test_sig5;
                can_data.test_msg5_2.test_sig5_2 = (int16_t) msg_data_a->test_msg5_2.test_sig5_2;
                can_data.test_msg5_2.test_sig5_3 = UINT32_TO_FLOAT(msg_data_a->test_msg5_2.test_sig5_3);
                can_data.test_msg5_2.stale = 0;
                can_data.test_msg5_2.last_rx = sched.os_ticks;
                break;
            case ID_TEST_STALE:
                can_data.test_stale.data = msg_data_a->test_stale.data;
                can_data.test_stale.stale = 0;
                can_data.test_stale.last_rx = sched.os_ticks;
                break;
            case ID_CAR_STATE2:
                can_data.car_state2.car_state2 = msg_data_a->car_state2.car_state2;
                break;
            case ID_L4_TESTING_BL_CMD:
                can_data.l4_testing_bl_cmd.cmd = msg_data_a->l4_testing_bl_cmd.cmd;
                can_data.l4_testing_bl_cmd.data = msg_data_a->l4_testing_bl_cmd.data;
                l4_testing_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_FAULT_SYNC_PDU:
                can_data.fault_sync_pdu.idx = msg_data_a->fault_sync_pdu.idx;
                can_data.fault_sync_pdu.latched = msg_data_a->fault_sync_pdu.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_MAIN_MODULE:
                can_data.fault_sync_main_module.idx = msg_data_a->fault_sync_main_module.idx;
                can_data.fault_sync_main_module.latched = msg_data_a->fault_sync_main_module.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_DASHBOARD:
                can_data.fault_sync_dashboard.idx = msg_data_a->fault_sync_dashboard.idx;
                can_data.fault_sync_dashboard.latched = msg_data_a->fault_sync_dashboard.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_A_BOX:
                can_data.fault_sync_a_box.idx = msg_data_a->fault_sync_a_box.idx;
                can_data.fault_sync_a_box.latched = msg_data_a->fault_sync_a_box.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_FAULT_SYNC_TORQUE_VECTOR:
                can_data.fault_sync_torque_vector.idx = msg_data_a->fault_sync_torque_vector.idx;
                can_data.fault_sync_torque_vector.latched = msg_data_a->fault_sync_torque_vector.latched;
				handleCallbacks(msg_data_a->fault_sync_main_module.idx, msg_data_a->fault_sync_main_module.latched);
                break;
            case ID_SET_FAULT:
                can_data.set_fault.id = msg_data_a->set_fault.id;
                can_data.set_fault.value = msg_data_a->set_fault.value;
				set_fault_daq(msg_data_a->set_fault.id, msg_data_a->set_fault.value);
                break;
            case ID_RETURN_FAULT_CONTROL:
                can_data.return_fault_control.id = msg_data_a->return_fault_control.id;
				return_fault_control(msg_data_a->return_fault_control.id);
                break;
            case ID_DAQ_COMMAND_TEST_NODE_TEST:
                can_data.daq_command_TEST_NODE_TEST.daq_command = msg_data_a->daq_command_TEST_NODE_TEST.daq_command;
                daq_command_TEST_NODE_TEST_CALLBACK(&msg_header);
                break;
            default:
                __asm__("nop");
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.test_msg5_2.stale,
                sched.os_ticks, can_data.test_msg5_2.last_rx,
                UP_TEST_MSG5_2);
    CHECK_STALE(can_data.test_stale.stale,
                sched.os_ticks, can_data.test_stale.last_rx,
                UP_TEST_STALE);
    /* END AUTO STALE CHECKS */
}

bool initCANFilter()
{
    CAN1->MCR |= CAN_MCR_INRQ;                // Enter back into INIT state (required for changing scale)
    uint32_t timeout = 0;
    while(!(CAN1->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
         ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
         return false;

    CAN1->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    CAN1->FM1R |= 0x07FFFFFF;                 // Set banks 0-27 to id mode
    CAN1->FS1R |= 0x07FFFFFF;                 // Set banks 0-27 to 32-bit scale

    /* BEGIN AUTO FILTER */
    CAN1->FA1R |= (1 << 0);    // configure bank 0
    CAN1->sFilterRegister[0].FR1 = (ID_TEST_MSG5_2 << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_TEST_STALE << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_CAR_STATE2 << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_L4_TESTING_BL_CMD << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_FAULT_SYNC_PDU << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_FAULT_SYNC_MAIN_MODULE << 3) | 4;
    CAN1->FA1R |= (1 << 3);    // configure bank 3
    CAN1->sFilterRegister[3].FR1 = (ID_FAULT_SYNC_DASHBOARD << 3) | 4;
    CAN1->sFilterRegister[3].FR2 = (ID_FAULT_SYNC_A_BOX << 3) | 4;
    CAN1->FA1R |= (1 << 4);    // configure bank 4
    CAN1->sFilterRegister[4].FR1 = (ID_FAULT_SYNC_TORQUE_VECTOR << 3) | 4;
    CAN1->sFilterRegister[4].FR2 = (ID_SET_FAULT << 3) | 4;
    CAN1->FA1R |= (1 << 5);    // configure bank 5
    CAN1->sFilterRegister[5].FR1 = (ID_RETURN_FAULT_CONTROL << 3) | 4;
    CAN1->sFilterRegister[5].FR2 = (ID_DAQ_COMMAND_TEST_NODE_TEST << 3) | 4;
    /* END AUTO FILTER */

    CAN1->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters (exit filter init mode)

    // Enter back into NORMAL mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while((CAN1->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
         ;

    return timeout != PHAL_CAN_INIT_TIMEOUT;
}


void canProcessRxIRQs(CanMsgTypeDef_t* rx)
{
    CanParsedData_t* msg_data_a;

    msg_data_a = (CanParsedData_t *) rx->Data;
    switch(rx->ExtId)
    {
        /* BEGIN AUTO RX IRQ */
        /* END AUTO RX IRQ */
        default:
            __asm__("nop");
    }
}
