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
            case ID_MAIN_HB:
                can_data.main_hb.car_state = msg_data_a->main_hb.car_state;
                can_data.main_hb.precharge_state = msg_data_a->main_hb.precharge_state;
                can_data.main_hb.stale = 0;
                can_data.main_hb.last_rx = sched.os_ticks;
                break;
            case ID_FRONT_WHEEL_DATA:
                can_data.front_wheel_data.left_speed = msg_data_a->front_wheel_data.left_speed;
                can_data.front_wheel_data.right_speed = msg_data_a->front_wheel_data.right_speed;
                can_data.front_wheel_data.left_normal = msg_data_a->front_wheel_data.left_normal;
                can_data.front_wheel_data.right_normal = msg_data_a->front_wheel_data.right_normal;
                can_data.front_wheel_data.stale = 0;
                can_data.front_wheel_data.last_rx = sched.os_ticks;
                break;
            case ID_FRONT_MOTOR_CURRENTS_TEMPS:
                can_data.front_motor_currents_temps.left_current = msg_data_a->front_motor_currents_temps.left_current;
                can_data.front_motor_currents_temps.right_current = msg_data_a->front_motor_currents_temps.right_current;
                can_data.front_motor_currents_temps.left_temp = msg_data_a->front_motor_currents_temps.left_temp;
                can_data.front_motor_currents_temps.right_temp = msg_data_a->front_motor_currents_temps.right_temp;
                can_data.front_motor_currents_temps.stale = 0;
                can_data.front_motor_currents_temps.last_rx = sched.os_ticks;
                break;
            case ID_REAR_MOTOR_CURRENTS_TEMPS:
                can_data.rear_motor_currents_temps.left_current = msg_data_a->rear_motor_currents_temps.left_current;
                can_data.rear_motor_currents_temps.right_current = msg_data_a->rear_motor_currents_temps.right_current;
                can_data.rear_motor_currents_temps.left_temp = msg_data_a->rear_motor_currents_temps.left_temp;
                can_data.rear_motor_currents_temps.right_temp = msg_data_a->rear_motor_currents_temps.right_temp;
                can_data.rear_motor_currents_temps.stale = 0;
                can_data.rear_motor_currents_temps.last_rx = sched.os_ticks;
                break;
            case ID_FRONT_DRIVELINE_HB:
                can_data.front_driveline_hb.front_left_motor = msg_data_a->front_driveline_hb.front_left_motor;
                can_data.front_driveline_hb.front_right_motor = msg_data_a->front_driveline_hb.front_right_motor;
                can_data.front_driveline_hb.stale = 0;
                can_data.front_driveline_hb.last_rx = sched.os_ticks;
                break;
            case ID_REAR_DRIVELINE_HB:
                can_data.rear_driveline_hb.rear_left_motor = msg_data_a->rear_driveline_hb.rear_left_motor;
                can_data.rear_driveline_hb.rear_right_motor = msg_data_a->rear_driveline_hb.rear_right_motor;
                can_data.rear_driveline_hb.stale = 0;
                can_data.rear_driveline_hb.last_rx = sched.os_ticks;
                break;
            case ID_TORQUE_REQUEST_MAIN:
                can_data.torque_request_main.front_left = (int16_t) msg_data_a->torque_request_main.front_left;
                can_data.torque_request_main.front_right = (int16_t) msg_data_a->torque_request_main.front_right;
                can_data.torque_request_main.rear_left = (int16_t) msg_data_a->torque_request_main.rear_left;
                can_data.torque_request_main.rear_right = (int16_t) msg_data_a->torque_request_main.rear_right;
                can_data.torque_request_main.stale = 0;
                can_data.torque_request_main.last_rx = sched.os_ticks;
                break;
            case ID_SOC_CELLS:
                can_data.soc_cells.idx = msg_data_a->soc_cells.idx;
                can_data.soc_cells.soc1 = msg_data_a->soc_cells.soc1;
                can_data.soc_cells.soc2 = msg_data_a->soc_cells.soc2;
                can_data.soc_cells.soc3 = msg_data_a->soc_cells.soc3;
                break;
            case ID_VOLTS_CELLS:
                can_data.volts_cells.idx = msg_data_a->volts_cells.idx;
                can_data.volts_cells.v1 = msg_data_a->volts_cells.v1;
                can_data.volts_cells.v2 = msg_data_a->volts_cells.v2;
                can_data.volts_cells.v3 = msg_data_a->volts_cells.v3;
                break;
            case ID_PACK_INFO:
                can_data.pack_info.volts = msg_data_a->pack_info.volts;
                can_data.pack_info.error = msg_data_a->pack_info.error;
                can_data.pack_info.bal_flags = msg_data_a->pack_info.bal_flags;
                break;
            case ID_TEMPS_CELLS:
                can_data.temps_cells.idx = msg_data_a->temps_cells.idx;
                can_data.temps_cells.t1 = msg_data_a->temps_cells.t1;
                can_data.temps_cells.t2 = msg_data_a->temps_cells.t2;
                can_data.temps_cells.t3 = msg_data_a->temps_cells.t3;
                break;
            case ID_CELL_INFO:
                can_data.cell_info.delta = msg_data_a->cell_info.delta;
                can_data.cell_info.ov = msg_data_a->cell_info.ov;
                can_data.cell_info.uv = msg_data_a->cell_info.uv;
                break;
            case ID_POWER_LIM:
                can_data.power_lim.disch_lim = msg_data_a->power_lim.disch_lim;
                can_data.power_lim.chg_lim = msg_data_a->power_lim.chg_lim;
                break;
            case ID_REAR_WHEEL_DATA:
                can_data.rear_wheel_data.left_speed = msg_data_a->rear_wheel_data.left_speed;
                can_data.rear_wheel_data.right_speed = msg_data_a->rear_wheel_data.right_speed;
                can_data.rear_wheel_data.left_normal = msg_data_a->rear_wheel_data.left_normal;
                can_data.rear_wheel_data.right_normal = msg_data_a->rear_wheel_data.right_normal;
                can_data.rear_wheel_data.stale = 0;
                can_data.rear_wheel_data.last_rx = sched.os_ticks;
                break;
            case ID_DAQ_COMMAND_DASHBOARD:
                can_data.daq_command_DASHBOARD.daq_command = msg_data_a->daq_command_DASHBOARD.daq_command;
                daq_command_DASHBOARD_CALLBACK(&msg_header);
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
    CHECK_STALE(can_data.front_wheel_data.stale,
                sched.os_ticks, can_data.front_wheel_data.last_rx,
                UP_FRONT_WHEEL_DATA);
    CHECK_STALE(can_data.front_motor_currents_temps.stale,
                sched.os_ticks, can_data.front_motor_currents_temps.last_rx,
                UP_FRONT_MOTOR_CURRENTS_TEMPS);
    CHECK_STALE(can_data.rear_motor_currents_temps.stale,
                sched.os_ticks, can_data.rear_motor_currents_temps.last_rx,
                UP_REAR_MOTOR_CURRENTS_TEMPS);
    CHECK_STALE(can_data.front_driveline_hb.stale,
                sched.os_ticks, can_data.front_driveline_hb.last_rx,
                UP_FRONT_DRIVELINE_HB);
    CHECK_STALE(can_data.rear_driveline_hb.stale,
                sched.os_ticks, can_data.rear_driveline_hb.last_rx,
                UP_REAR_DRIVELINE_HB);
    CHECK_STALE(can_data.torque_request_main.stale,
                sched.os_ticks, can_data.torque_request_main.last_rx,
                UP_TORQUE_REQUEST_MAIN);
    CHECK_STALE(can_data.rear_wheel_data.stale,
                sched.os_ticks, can_data.rear_wheel_data.last_rx,
                UP_REAR_WHEEL_DATA);
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
    CAN1->sFilterRegister[0].FR1 = (ID_MAIN_HB << 3) | 4;
    CAN1->sFilterRegister[0].FR2 = (ID_FRONT_WHEEL_DATA << 3) | 4;
    CAN1->FA1R |= (1 << 1);    // configure bank 1
    CAN1->sFilterRegister[1].FR1 = (ID_FRONT_MOTOR_CURRENTS_TEMPS << 3) | 4;
    CAN1->sFilterRegister[1].FR2 = (ID_REAR_MOTOR_CURRENTS_TEMPS << 3) | 4;
    CAN1->FA1R |= (1 << 2);    // configure bank 2
    CAN1->sFilterRegister[2].FR1 = (ID_FRONT_DRIVELINE_HB << 3) | 4;
    CAN1->sFilterRegister[2].FR2 = (ID_REAR_DRIVELINE_HB << 3) | 4;
    CAN1->FA1R |= (1 << 3);    // configure bank 3
    CAN1->sFilterRegister[3].FR1 = (ID_TORQUE_REQUEST_MAIN << 3) | 4;
    CAN1->sFilterRegister[3].FR2 = (ID_SOC_CELLS << 3) | 4;
    CAN1->FA1R |= (1 << 4);    // configure bank 4
    CAN1->sFilterRegister[4].FR1 = (ID_VOLTS_CELLS << 3) | 4;
    CAN1->sFilterRegister[4].FR2 = (ID_PACK_INFO << 3) | 4;
    CAN1->FA1R |= (1 << 5);    // configure bank 5
    CAN1->sFilterRegister[5].FR1 = (ID_TEMPS_CELLS << 3) | 4;
    CAN1->sFilterRegister[5].FR2 = (ID_CELL_INFO << 3) | 4;
    CAN1->FA1R |= (1 << 6);    // configure bank 6
    CAN1->sFilterRegister[6].FR1 = (ID_POWER_LIM << 3) | 4;
    CAN1->sFilterRegister[6].FR2 = (ID_REAR_WHEEL_DATA << 3) | 4;
    CAN1->FA1R |= (1 << 7);    // configure bank 7
    CAN1->sFilterRegister[7].FR1 = (ID_DAQ_COMMAND_DASHBOARD << 3) | 4;
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
