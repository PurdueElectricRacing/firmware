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

uint32_t curr_tick = 0;

void canRxUpdate()
{
    curr_tick += 1;

    CanMsgTypeDef_t msg_header;
    CanParsedData_t* msg_data_a;

    if(qReceive(q_rx_can_a, &msg_header) == SUCCESS_G)
    {
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        switch(msg_header.ExtId)
        {
            /* BEGIN AUTO CASES */
            case ID_THROTTLE_BRAKE:
                can_data.throttle_brake.raw_throttle = msg_data_a->throttle_brake.raw_throttle;
                can_data.throttle_brake.raw_brake = msg_data_a->throttle_brake.raw_brake;
                can_data.throttle_brake.stale = 0;
                can_data.throttle_brake.last_rx = curr_tick;
                break;
            case ID_WHEEL_SPEEDS:
                can_data.wheel_speeds.fl_speed = msg_data_a->wheel_speeds.fl_speed;
                can_data.wheel_speeds.fr_speed = msg_data_a->wheel_speeds.fr_speed;
                can_data.wheel_speeds.bl_speed = msg_data_a->wheel_speeds.bl_speed;
                can_data.wheel_speeds.br_speed = msg_data_a->wheel_speeds.br_speed;
                can_data.wheel_speeds.stale = 0;
                can_data.wheel_speeds.last_rx = curr_tick;
                break;
            case ID_MOTOR_CURRENT:
                can_data.motor_current.current = msg_data_a->motor_current.current;
                can_data.motor_current.stale = 0;
                can_data.motor_current.last_rx = curr_tick;
                break;
            /* END AUTO CASES */
            default:            // ID wasn't recognized
                __asm__("nop"); // Do nothing so we can place a breakpoint
        }
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.throttle_brake.stale,
                curr_tick, can_data.throttle_brake.last_rx,
                UP_THROTTLE_BRAKE);
    CHECK_STALE(can_data.wheel_speeds.stale,
                curr_tick, can_data.wheel_speeds.last_rx,
                UP_WHEEL_SPEEDS);
    CHECK_STALE(can_data.motor_current.stale,
                curr_tick, can_data.motor_current.last_rx,
                UP_MOTOR_CURRENT);
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
    CAN1->sFilterRegister[0].FR1 = (ID_THROTTLE_BRAKE << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_WHEEL_SPEEDS << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_MOTOR_CURRENT << 3) | 4;
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
