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
uint32_t last_can_rx_time_ms = 0;

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
        last_can_rx_time_ms = sched.os_ticks;
        /* BEGIN AUTO CASES */
        switch(msg_header.ExtId)
        {
            case ID_TORQUE_REQUEST_MAIN:
                can_data.torque_request_main.front_left = (int16_t) msg_data_a->torque_request_main.front_left;
                can_data.torque_request_main.front_right = (int16_t) msg_data_a->torque_request_main.front_right;
                can_data.torque_request_main.rear_left = (int16_t) msg_data_a->torque_request_main.rear_left;
                can_data.torque_request_main.rear_right = (int16_t) msg_data_a->torque_request_main.rear_right;
                can_data.torque_request_main.stale = 0;
                can_data.torque_request_main.last_rx = sched.os_ticks;
                break;
            case ID_MAIN_HB:
                can_data.main_hb.car_state = msg_data_a->main_hb.car_state;
                can_data.main_hb.precharge_state = msg_data_a->main_hb.precharge_state;
                can_data.main_hb.stale = 0;
                can_data.main_hb.last_rx = sched.os_ticks;
                break;
            case ID_DRIVELINE_FRONT_BL_CMD:
                can_data.driveline_front_bl_cmd.cmd = msg_data_a->driveline_front_bl_cmd.cmd;
                can_data.driveline_front_bl_cmd.data = msg_data_a->driveline_front_bl_cmd.data;
                driveline_front_bl_cmd_CALLBACK(msg_data_a);
                break;
            case ID_DRIVELINE_REAR_BL_CMD:
                can_data.driveline_rear_bl_cmd.cmd = msg_data_a->driveline_rear_bl_cmd.cmd;
                can_data.driveline_rear_bl_cmd.data = msg_data_a->driveline_rear_bl_cmd.data;
                driveline_rear_bl_cmd_CALLBACK(msg_data_a);
                break;
            default:
                __asm__("nop");
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.torque_request_main.stale,
                sched.os_ticks, can_data.torque_request_main.last_rx,
                UP_TORQUE_REQUEST_MAIN);
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
    CAN1->sFilterRegister[0].FR1 = (ID_TORQUE_REQUEST_MAIN << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_MAIN_HB << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_DRIVELINE_FRONT_BL_CMD << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_DRIVELINE_REAR_BL_CMD << 3) | 4;
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
