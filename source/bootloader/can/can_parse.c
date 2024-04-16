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

void canRxUpdate()
{
    CanMsgTypeDef_t msg_header;
    CanParsedData_t* msg_data_a;

    if(qReceive(q_rx_can_a, &msg_header) == SUCCESS_G)
    {
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        /* BEGIN AUTO CASES */
        switch(msg_header.ExtId)
        {
            case ID_BITSTREAM_DATA:
                can_data.bitstream_data.d0 = msg_data_a->bitstream_data.d0;
                can_data.bitstream_data.d1 = msg_data_a->bitstream_data.d1;
                can_data.bitstream_data.d2 = msg_data_a->bitstream_data.d2;
                can_data.bitstream_data.d3 = msg_data_a->bitstream_data.d3;
                can_data.bitstream_data.d4 = msg_data_a->bitstream_data.d4;
                can_data.bitstream_data.d5 = msg_data_a->bitstream_data.d5;
                can_data.bitstream_data.d6 = msg_data_a->bitstream_data.d6;
                can_data.bitstream_data.d7 = msg_data_a->bitstream_data.d7;
                bitstream_data_CALLBACK(msg_data_a);
                break;
            case ID_MAIN_MODULE_BL_CMD:
                can_data.main_module_bl_cmd.cmd = msg_data_a->main_module_bl_cmd.cmd;
                can_data.main_module_bl_cmd.data = msg_data_a->main_module_bl_cmd.data;
                main_module_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_DASHBOARD_BL_CMD:
                can_data.dashboard_bl_cmd.cmd = msg_data_a->dashboard_bl_cmd.cmd;
                can_data.dashboard_bl_cmd.data = msg_data_a->dashboard_bl_cmd.data;
                dashboard_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_TORQUEVECTOR_BL_CMD:
                can_data.torquevector_bl_cmd.cmd = msg_data_a->torquevector_bl_cmd.cmd;
                can_data.torquevector_bl_cmd.data = msg_data_a->torquevector_bl_cmd.data;
                torquevector_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_A_BOX_BL_CMD:
                can_data.a_box_bl_cmd.cmd = msg_data_a->a_box_bl_cmd.cmd;
                can_data.a_box_bl_cmd.data = msg_data_a->a_box_bl_cmd.data;
                a_box_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_PDU_BL_CMD:
                can_data.pdu_bl_cmd.cmd = msg_data_a->pdu_bl_cmd.cmd;
                can_data.pdu_bl_cmd.data = msg_data_a->pdu_bl_cmd.data;
                pdu_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_L4_TESTING_BL_CMD:
                can_data.l4_testing_bl_cmd.cmd = msg_data_a->l4_testing_bl_cmd.cmd;
                can_data.l4_testing_bl_cmd.data = msg_data_a->l4_testing_bl_cmd.data;
                l4_testing_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_F4_TESTING_BL_CMD:
                can_data.f4_testing_bl_cmd.cmd = msg_data_a->f4_testing_bl_cmd.cmd;
                can_data.f4_testing_bl_cmd.data = msg_data_a->f4_testing_bl_cmd.data;
                f4_testing_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_F7_TESTING_BL_CMD:
                can_data.f7_testing_bl_cmd.cmd = msg_data_a->f7_testing_bl_cmd.cmd;
                can_data.f7_testing_bl_cmd.data = msg_data_a->f7_testing_bl_cmd.data;
                f7_testing_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_DAQ_BL_CMD:
                can_data.daq_bl_cmd.cmd = msg_data_a->daq_bl_cmd.cmd;
                can_data.daq_bl_cmd.data = msg_data_a->daq_bl_cmd.data;
                daq_bl_cmd_CALLBACK(msg_data_a);
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
#ifdef CAN2
    CAN2->MCR |= CAN_MCR_INRQ; 
#endif /* CAN2 */             
    uint32_t timeout = 0;
    while( !(CAN1->MSR & CAN_MSR_INAK)
#ifdef CAN2
        &&   !(CAN2->MSR & CAN_MSR_INAK)
#endif /* CAN2 */
           && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
         return false;

    CAN1->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    CAN1->FM1R |= 0x07FFFFFF;                 // Set banks 0-27 to id mode
    CAN1->FS1R |= 0x07FFFFFF;                 // Set banks 0-27 to 32-bit scale
#ifdef CAN2
    CAN2->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    CAN2->FM1R |= 0x07FFFFFF;                 // Set banks 0-27 to id mode
    CAN2->FS1R |= 0x07FFFFFF;                 // Set banks 0-27 to 32-bit scale
#endif /* CAN2 */
    /* BEGIN AUTO FILTER */
    CAN1->FA1R |= (1 << 0);    // configure bank 0
    CAN1->sFilterRegister[0].FR1 = (ID_BITSTREAM_DATA << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_MAIN_MODULE_BL_CMD << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_DASHBOARD_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_TORQUEVECTOR_BL_CMD << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_A_BOX_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_PDU_BL_CMD << 3) | 4;
    CAN1->FA1R |= (1 << 3);    // configure bank 3
    CAN1->sFilterRegister[3].FR1 = (ID_L4_TESTING_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[3].FR2 = (ID_F4_TESTING_BL_CMD << 3) | 4;
    CAN1->FA1R |= (1 << 4);    // configure bank 4
    CAN1->sFilterRegister[4].FR1 = (ID_F7_TESTING_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[4].FR2 = (ID_DAQ_BL_CMD << 3) | 4;
    /* END AUTO FILTER */

    CAN1->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters (exit filter init mode)

    // Enter back into NORMAL mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
#ifdef CAN2
    CAN2->MCR &= ~CAN_MCR_INRQ;
#endif /* CAN2 */
    while((CAN1->MSR & CAN_MSR_INAK)
#ifdef CAN2 
        &&  (CAN2->MSR & CAN_MSR_INAK)
#endif /* CAN2 */
        && ++timeout < PHAL_CAN_INIT_TIMEOUT)
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
