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
volatile uint32_t last_can_rx_time_ms = 0;

void initCANParse(void)
{
    initCANParseBase();
    initCANFilter();
}

void canRxUpdate()
{
    CanMsgTypeDef_t msg_header;
    CanParsedData_t* msg_data_a;

    if(qReceive(&q_rx_can, &msg_header) == SUCCESS_G)
    {
        last_can_rx_time_ms = sched.os_ticks;
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        /* BEGIN AUTO CASES */
        switch(msg_header.ExtId)
        {
            case ID_PDU_BL_CMD:
                can_data.pdu_bl_cmd.cmd = msg_data_a->pdu_bl_cmd.cmd;
                can_data.pdu_bl_cmd.data = msg_data_a->pdu_bl_cmd.data;
                pdu_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_COOLING_DRIVER_REQUEST:
                can_data.cooling_driver_request.dt_pump = msg_data_a->cooling_driver_request.dt_pump;
                can_data.cooling_driver_request.dt_fan = msg_data_a->cooling_driver_request.dt_fan;
                can_data.cooling_driver_request.batt_pump = msg_data_a->cooling_driver_request.batt_pump;
                can_data.cooling_driver_request.batt_pump2 = msg_data_a->cooling_driver_request.batt_pump2;
                can_data.cooling_driver_request.batt_fan = msg_data_a->cooling_driver_request.batt_fan;
                cooling_driver_request_CALLBACK(msg_data_a);
                break;
            case ID_MAIN_HB:
                can_data.main_hb.car_state = msg_data_a->main_hb.car_state;
                can_data.main_hb.precharge_state = msg_data_a->main_hb.precharge_state;
                can_data.main_hb.stale = 0;
                can_data.main_hb.last_rx = sched.os_ticks;
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
            case ID_FAULT_SYNC_TEST_NODE:
                can_data.fault_sync_test_node.idx = msg_data_a->fault_sync_test_node.idx;
                can_data.fault_sync_test_node.latched = msg_data_a->fault_sync_test_node.latched;
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
            case ID_DAQ_COMMAND_PDU:
                can_data.daq_command_PDU.daq_command = msg_data_a->daq_command_PDU.daq_command;
                daq_command_PDU_CALLBACK(&msg_header);
                break;
            default:
                __asm__("nop");
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.main_hb.stale,
                sched.os_ticks, can_data.main_hb.last_rx,
                UP_MAIN_HB);
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
    CAN1->sFilterRegister[0].FR1 = (ID_PDU_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_COOLING_DRIVER_REQUEST << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_MAIN_HB << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_FAULT_SYNC_MAIN_MODULE << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_FAULT_SYNC_DASHBOARD << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_FAULT_SYNC_A_BOX << 3) | 4;
    CAN1->FA1R |= (1 << 3);    // configure bank 3
    CAN1->sFilterRegister[3].FR1 = (ID_FAULT_SYNC_TORQUE_VECTOR << 3) | 4;
    CAN1->sFilterRegister[3].FR2 = (ID_FAULT_SYNC_TEST_NODE << 3) | 4;
    CAN1->FA1R |= (1 << 4);    // configure bank 4
    CAN1->sFilterRegister[4].FR1 = (ID_SET_FAULT << 3) | 4;
    CAN1->sFilterRegister[4].FR2 = (ID_RETURN_FAULT_CONTROL << 3) | 4;
    CAN1->FA1R |= (1 << 5);    // configure bank 5
    CAN1->sFilterRegister[5].FR1 = (ID_DAQ_COMMAND_PDU << 3) | 4;
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
