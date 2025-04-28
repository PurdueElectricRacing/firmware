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

#if 0
// prototypes
bool initCANFilter();

can_data_t can_data;
volatile uint32_t last_can_rx_time_ms = 0;

void initCANParse(void)
{
    initCANParseBase();
    initCANFilter();
}

void canRxUpdate(void)
{
    CanMsgTypeDef_t msg_header;
    CanParsedData_t* msg_data_a;

    if(qReceive(&q_rx_can, &msg_header) == SUCCESS_G)
    {
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        last_can_rx_time_ms = sched.os_ticks;
        /* BEGIN AUTO CASES */
        switch(msg_header.ExtId)
        {
            case ID_UDS_RESPONSE_MAIN_MODULE:
                can_data.uds_response_main_module.payload = msg_data_a->uds_response_main_module.payload;
                break;
            case ID_UDS_RESPONSE_DASHBOARD:
                can_data.uds_response_dashboard.payload = msg_data_a->uds_response_dashboard.payload;
                break;
            case ID_UDS_RESPONSE_A_BOX:
                can_data.uds_response_a_box.payload = msg_data_a->uds_response_a_box.payload;
                break;
            case ID_UDS_RESPONSE_PDU:
                can_data.uds_response_pdu.payload = msg_data_a->uds_response_pdu.payload;
                break;
            case ID_UDS_COMMAND_DAQ:
                can_data.uds_command_daq.payload = msg_data_a->uds_command_daq.payload;
				uds_command_daq_CALLBACK(msg_data_a->uds_command_daq.payload);
                break;
            case ID_UDS_RESPONSE_TORQUE_VECTOR:
                can_data.uds_response_torque_vector.payload = msg_data_a->uds_response_torque_vector.payload;
                break;
            default:
                __asm__("nop");
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
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
    CAN1->sFilterRegister[0].FR1 = (ID_UDS_RESPONSE_MAIN_MODULE << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_UDS_RESPONSE_DASHBOARD << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_UDS_RESPONSE_A_BOX << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_UDS_RESPONSE_PDU << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_UDS_COMMAND_DAQ << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_UDS_RESPONSE_TORQUE_VECTOR << 3) | 4;
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
#endif
