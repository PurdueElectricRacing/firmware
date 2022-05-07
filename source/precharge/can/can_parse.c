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
volatile uint32_t last_can_rx_time_ms = 0;

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
        last_can_rx_time_ms = sched.os_ticks;
        msg_data_a = (CanParsedData_t *) &msg_header.Data;
        /* BEGIN AUTO CASES */
        if (msg_header.Bus == CAN2)
        {
            switch(msg_header.ExtId)
            {
                case ID_SOC_CELLS_1:
                    can_data.soc_cells_1.idx = msg_data_a->soc_cells_1.idx;
                    can_data.soc_cells_1.soc1 = msg_data_a->soc_cells_1.soc1;
                    can_data.soc_cells_1.soc2 = msg_data_a->soc_cells_1.soc2;
                    can_data.soc_cells_1.soc3 = msg_data_a->soc_cells_1.soc3;
                    break;
                case ID_VOLTS_CELLS_1:
                    can_data.volts_cells_1.idx = msg_data_a->volts_cells_1.idx;
                    can_data.volts_cells_1.v1 = msg_data_a->volts_cells_1.v1;
                    can_data.volts_cells_1.v2 = msg_data_a->volts_cells_1.v2;
                    can_data.volts_cells_1.v3 = msg_data_a->volts_cells_1.v3;
                    volts_cells_1_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_1:
                    can_data.pack_info_1.volts = msg_data_a->pack_info_1.volts;
                    can_data.pack_info_1.error = msg_data_a->pack_info_1.error;
                    can_data.pack_info_1.bal_flags = msg_data_a->pack_info_1.bal_flags;
                    break;
                case ID_TEMPS_CELLS_1:
                    can_data.temps_cells_1.idx = msg_data_a->temps_cells_1.idx;
                    can_data.temps_cells_1.t1 = msg_data_a->temps_cells_1.t1;
                    can_data.temps_cells_1.t2 = msg_data_a->temps_cells_1.t2;
                    can_data.temps_cells_1.t3 = msg_data_a->temps_cells_1.t3;
                    break;
                case ID_CELL_INFO_1:
                    can_data.cell_info_1.delta = msg_data_a->cell_info_1.delta;
                    can_data.cell_info_1.ov = msg_data_a->cell_info_1.ov;
                    can_data.cell_info_1.uv = msg_data_a->cell_info_1.uv;
                    break;
                case ID_POWER_LIM_1:
                    can_data.power_lim_1.disch_lim = msg_data_a->power_lim_1.disch_lim;
                    can_data.power_lim_1.chg_lim = msg_data_a->power_lim_1.chg_lim;
                    break;
                case ID_SOC_CELLS_2:
                    can_data.soc_cells_2.idx = msg_data_a->soc_cells_2.idx;
                    can_data.soc_cells_2.soc1 = msg_data_a->soc_cells_2.soc1;
                    can_data.soc_cells_2.soc2 = msg_data_a->soc_cells_2.soc2;
                    can_data.soc_cells_2.soc3 = msg_data_a->soc_cells_2.soc3;
                    break;
                case ID_VOLTS_CELLS_2:
                    can_data.volts_cells_2.idx = msg_data_a->volts_cells_2.idx;
                    can_data.volts_cells_2.v1 = msg_data_a->volts_cells_2.v1;
                    can_data.volts_cells_2.v2 = msg_data_a->volts_cells_2.v2;
                    can_data.volts_cells_2.v3 = msg_data_a->volts_cells_2.v3;
                    volts_cells_2_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_2:
                    can_data.pack_info_2.volts = msg_data_a->pack_info_2.volts;
                    can_data.pack_info_2.error = msg_data_a->pack_info_2.error;
                    can_data.pack_info_2.bal_flags = msg_data_a->pack_info_2.bal_flags;
                    break;
                case ID_TEMPS_CELLS_2:
                    can_data.temps_cells_2.idx = msg_data_a->temps_cells_2.idx;
                    can_data.temps_cells_2.t1 = msg_data_a->temps_cells_2.t1;
                    can_data.temps_cells_2.t2 = msg_data_a->temps_cells_2.t2;
                    can_data.temps_cells_2.t3 = msg_data_a->temps_cells_2.t3;
                    break;
                case ID_CELL_INFO_2:
                    can_data.cell_info_2.delta = msg_data_a->cell_info_2.delta;
                    can_data.cell_info_2.ov = msg_data_a->cell_info_2.ov;
                    can_data.cell_info_2.uv = msg_data_a->cell_info_2.uv;
                    break;
                case ID_POWER_LIM_2:
                    can_data.power_lim_2.disch_lim = msg_data_a->power_lim_2.disch_lim;
                    can_data.power_lim_2.chg_lim = msg_data_a->power_lim_2.chg_lim;
                    break;
                case ID_SOC_CELLS_3:
                    can_data.soc_cells_3.idx = msg_data_a->soc_cells_3.idx;
                    can_data.soc_cells_3.soc1 = msg_data_a->soc_cells_3.soc1;
                    can_data.soc_cells_3.soc2 = msg_data_a->soc_cells_3.soc2;
                    can_data.soc_cells_3.soc3 = msg_data_a->soc_cells_3.soc3;
                    break;
                case ID_VOLTS_CELLS_3:
                    can_data.volts_cells_3.idx = msg_data_a->volts_cells_3.idx;
                    can_data.volts_cells_3.v1 = msg_data_a->volts_cells_3.v1;
                    can_data.volts_cells_3.v2 = msg_data_a->volts_cells_3.v2;
                    can_data.volts_cells_3.v3 = msg_data_a->volts_cells_3.v3;
                    volts_cells_3_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_3:
                    can_data.pack_info_3.volts = msg_data_a->pack_info_3.volts;
                    can_data.pack_info_3.error = msg_data_a->pack_info_3.error;
                    can_data.pack_info_3.bal_flags = msg_data_a->pack_info_3.bal_flags;
                    break;
                case ID_TEMPS_CELLS_3:
                    can_data.temps_cells_3.idx = msg_data_a->temps_cells_3.idx;
                    can_data.temps_cells_3.t1 = msg_data_a->temps_cells_3.t1;
                    can_data.temps_cells_3.t2 = msg_data_a->temps_cells_3.t2;
                    can_data.temps_cells_3.t3 = msg_data_a->temps_cells_3.t3;
                    break;
                case ID_CELL_INFO_3:
                    can_data.cell_info_3.delta = msg_data_a->cell_info_3.delta;
                    can_data.cell_info_3.ov = msg_data_a->cell_info_3.ov;
                    can_data.cell_info_3.uv = msg_data_a->cell_info_3.uv;
                    break;
                case ID_POWER_LIM_3:
                    can_data.power_lim_3.disch_lim = msg_data_a->power_lim_3.disch_lim;
                    can_data.power_lim_3.chg_lim = msg_data_a->power_lim_3.chg_lim;
                    break;
                case ID_SOC_CELLS_4:
                    can_data.soc_cells_4.idx = msg_data_a->soc_cells_4.idx;
                    can_data.soc_cells_4.soc1 = msg_data_a->soc_cells_4.soc1;
                    can_data.soc_cells_4.soc2 = msg_data_a->soc_cells_4.soc2;
                    can_data.soc_cells_4.soc3 = msg_data_a->soc_cells_4.soc3;
                    break;
                case ID_VOLTS_CELLS_4:
                    can_data.volts_cells_4.idx = msg_data_a->volts_cells_4.idx;
                    can_data.volts_cells_4.v1 = msg_data_a->volts_cells_4.v1;
                    can_data.volts_cells_4.v2 = msg_data_a->volts_cells_4.v2;
                    can_data.volts_cells_4.v3 = msg_data_a->volts_cells_4.v3;
                    volts_cells_4_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_4:
                    can_data.pack_info_4.volts = msg_data_a->pack_info_4.volts;
                    can_data.pack_info_4.error = msg_data_a->pack_info_4.error;
                    can_data.pack_info_4.bal_flags = msg_data_a->pack_info_4.bal_flags;
                    break;
                case ID_TEMPS_CELLS_4:
                    can_data.temps_cells_4.idx = msg_data_a->temps_cells_4.idx;
                    can_data.temps_cells_4.t1 = msg_data_a->temps_cells_4.t1;
                    can_data.temps_cells_4.t2 = msg_data_a->temps_cells_4.t2;
                    can_data.temps_cells_4.t3 = msg_data_a->temps_cells_4.t3;
                    break;
                case ID_CELL_INFO_4:
                    can_data.cell_info_4.delta = msg_data_a->cell_info_4.delta;
                    can_data.cell_info_4.ov = msg_data_a->cell_info_4.ov;
                    can_data.cell_info_4.uv = msg_data_a->cell_info_4.uv;
                    break;
                case ID_POWER_LIM_4:
                    can_data.power_lim_4.disch_lim = msg_data_a->power_lim_4.disch_lim;
                    can_data.power_lim_4.chg_lim = msg_data_a->power_lim_4.chg_lim;
                    break;
                case ID_SOC_CELLS_5:
                    can_data.soc_cells_5.idx = msg_data_a->soc_cells_5.idx;
                    can_data.soc_cells_5.soc1 = msg_data_a->soc_cells_5.soc1;
                    can_data.soc_cells_5.soc2 = msg_data_a->soc_cells_5.soc2;
                    can_data.soc_cells_5.soc3 = msg_data_a->soc_cells_5.soc3;
                    break;
                case ID_VOLTS_CELLS_5:
                    can_data.volts_cells_5.idx = msg_data_a->volts_cells_5.idx;
                    can_data.volts_cells_5.v1 = msg_data_a->volts_cells_5.v1;
                    can_data.volts_cells_5.v2 = msg_data_a->volts_cells_5.v2;
                    can_data.volts_cells_5.v3 = msg_data_a->volts_cells_5.v3;
                    volts_cells_5_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_5:
                    can_data.pack_info_5.volts = msg_data_a->pack_info_5.volts;
                    can_data.pack_info_5.error = msg_data_a->pack_info_5.error;
                    can_data.pack_info_5.bal_flags = msg_data_a->pack_info_5.bal_flags;
                    break;
                case ID_TEMPS_CELLS_5:
                    can_data.temps_cells_5.idx = msg_data_a->temps_cells_5.idx;
                    can_data.temps_cells_5.t1 = msg_data_a->temps_cells_5.t1;
                    can_data.temps_cells_5.t2 = msg_data_a->temps_cells_5.t2;
                    can_data.temps_cells_5.t3 = msg_data_a->temps_cells_5.t3;
                    break;
                case ID_CELL_INFO_5:
                    can_data.cell_info_5.delta = msg_data_a->cell_info_5.delta;
                    can_data.cell_info_5.ov = msg_data_a->cell_info_5.ov;
                    can_data.cell_info_5.uv = msg_data_a->cell_info_5.uv;
                    break;
                case ID_POWER_LIM_5:
                    can_data.power_lim_5.disch_lim = msg_data_a->power_lim_5.disch_lim;
                    can_data.power_lim_5.chg_lim = msg_data_a->power_lim_5.chg_lim;
                    break;
                case ID_SOC_CELLS_6:
                    can_data.soc_cells_6.idx = msg_data_a->soc_cells_6.idx;
                    can_data.soc_cells_6.soc1 = msg_data_a->soc_cells_6.soc1;
                    can_data.soc_cells_6.soc2 = msg_data_a->soc_cells_6.soc2;
                    can_data.soc_cells_6.soc3 = msg_data_a->soc_cells_6.soc3;
                    break;
                case ID_VOLTS_CELLS_6:
                    can_data.volts_cells_6.idx = msg_data_a->volts_cells_6.idx;
                    can_data.volts_cells_6.v1 = msg_data_a->volts_cells_6.v1;
                    can_data.volts_cells_6.v2 = msg_data_a->volts_cells_6.v2;
                    can_data.volts_cells_6.v3 = msg_data_a->volts_cells_6.v3;
                    volts_cells_6_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_6:
                    can_data.pack_info_6.volts = msg_data_a->pack_info_6.volts;
                    can_data.pack_info_6.error = msg_data_a->pack_info_6.error;
                    can_data.pack_info_6.bal_flags = msg_data_a->pack_info_6.bal_flags;
                    break;
                case ID_TEMPS_CELLS_6:
                    can_data.temps_cells_6.idx = msg_data_a->temps_cells_6.idx;
                    can_data.temps_cells_6.t1 = msg_data_a->temps_cells_6.t1;
                    can_data.temps_cells_6.t2 = msg_data_a->temps_cells_6.t2;
                    can_data.temps_cells_6.t3 = msg_data_a->temps_cells_6.t3;
                    break;
                case ID_CELL_INFO_6:
                    can_data.cell_info_6.delta = msg_data_a->cell_info_6.delta;
                    can_data.cell_info_6.ov = msg_data_a->cell_info_6.ov;
                    can_data.cell_info_6.uv = msg_data_a->cell_info_6.uv;
                    break;
                case ID_POWER_LIM_6:
                    can_data.power_lim_6.disch_lim = msg_data_a->power_lim_6.disch_lim;
                    can_data.power_lim_6.chg_lim = msg_data_a->power_lim_6.chg_lim;
                    break;
                case ID_SOC_CELLS_7:
                    can_data.soc_cells_7.idx = msg_data_a->soc_cells_7.idx;
                    can_data.soc_cells_7.soc1 = msg_data_a->soc_cells_7.soc1;
                    can_data.soc_cells_7.soc2 = msg_data_a->soc_cells_7.soc2;
                    can_data.soc_cells_7.soc3 = msg_data_a->soc_cells_7.soc3;
                    break;
                case ID_VOLTS_CELLS_7:
                    can_data.volts_cells_7.idx = msg_data_a->volts_cells_7.idx;
                    can_data.volts_cells_7.v1 = msg_data_a->volts_cells_7.v1;
                    can_data.volts_cells_7.v2 = msg_data_a->volts_cells_7.v2;
                    can_data.volts_cells_7.v3 = msg_data_a->volts_cells_7.v3;
                    volts_cells_7_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_7:
                    can_data.pack_info_7.volts = msg_data_a->pack_info_7.volts;
                    can_data.pack_info_7.error = msg_data_a->pack_info_7.error;
                    can_data.pack_info_7.bal_flags = msg_data_a->pack_info_7.bal_flags;
                    break;
                case ID_TEMPS_CELLS_7:
                    can_data.temps_cells_7.idx = msg_data_a->temps_cells_7.idx;
                    can_data.temps_cells_7.t1 = msg_data_a->temps_cells_7.t1;
                    can_data.temps_cells_7.t2 = msg_data_a->temps_cells_7.t2;
                    can_data.temps_cells_7.t3 = msg_data_a->temps_cells_7.t3;
                    break;
                case ID_CELL_INFO_7:
                    can_data.cell_info_7.delta = msg_data_a->cell_info_7.delta;
                    can_data.cell_info_7.ov = msg_data_a->cell_info_7.ov;
                    can_data.cell_info_7.uv = msg_data_a->cell_info_7.uv;
                    break;
                case ID_POWER_LIM_7:
                    can_data.power_lim_7.disch_lim = msg_data_a->power_lim_7.disch_lim;
                    can_data.power_lim_7.chg_lim = msg_data_a->power_lim_7.chg_lim;
                    break;
                case ID_SOC_CELLS_8:
                    can_data.soc_cells_8.idx = msg_data_a->soc_cells_8.idx;
                    can_data.soc_cells_8.soc1 = msg_data_a->soc_cells_8.soc1;
                    can_data.soc_cells_8.soc2 = msg_data_a->soc_cells_8.soc2;
                    can_data.soc_cells_8.soc3 = msg_data_a->soc_cells_8.soc3;
                    break;
                case ID_VOLTS_CELLS_8:
                    can_data.volts_cells_8.idx = msg_data_a->volts_cells_8.idx;
                    can_data.volts_cells_8.v1 = msg_data_a->volts_cells_8.v1;
                    can_data.volts_cells_8.v2 = msg_data_a->volts_cells_8.v2;
                    can_data.volts_cells_8.v3 = msg_data_a->volts_cells_8.v3;
                    volts_cells_8_CALLBACK(msg_data_a);
                    break;
                case ID_PACK_INFO_8:
                    can_data.pack_info_8.volts = msg_data_a->pack_info_8.volts;
                    can_data.pack_info_8.error = msg_data_a->pack_info_8.error;
                    can_data.pack_info_8.bal_flags = msg_data_a->pack_info_8.bal_flags;
                    break;
                case ID_TEMPS_CELLS_8:
                    can_data.temps_cells_8.idx = msg_data_a->temps_cells_8.idx;
                    can_data.temps_cells_8.t1 = msg_data_a->temps_cells_8.t1;
                    can_data.temps_cells_8.t2 = msg_data_a->temps_cells_8.t2;
                    can_data.temps_cells_8.t3 = msg_data_a->temps_cells_8.t3;
                    break;
                case ID_CELL_INFO_8:
                    can_data.cell_info_8.delta = msg_data_a->cell_info_8.delta;
                    can_data.cell_info_8.ov = msg_data_a->cell_info_8.ov;
                    can_data.cell_info_8.uv = msg_data_a->cell_info_8.uv;
                    break;
                case ID_POWER_LIM_8:
                    can_data.power_lim_8.disch_lim = msg_data_a->power_lim_8.disch_lim;
                    can_data.power_lim_8.chg_lim = msg_data_a->power_lim_8.chg_lim;
                    break;
                default:
                    __asm__("nop");
            }
        }
        else if (msg_header.Bus == CAN1)
        {
            switch(msg_header.ExtId)
            {
                case ID_ELCON_CHARGER_STATUS:
                    can_data.elcon_charger_status.charge_voltage = msg_data_a->elcon_charger_status.charge_voltage;
                    can_data.elcon_charger_status.charge_current = msg_data_a->elcon_charger_status.charge_current;
                    can_data.elcon_charger_status.hw_fail = msg_data_a->elcon_charger_status.hw_fail;
                    can_data.elcon_charger_status.temp_fail = msg_data_a->elcon_charger_status.temp_fail;
                    can_data.elcon_charger_status.input_v_fail = msg_data_a->elcon_charger_status.input_v_fail;
                    can_data.elcon_charger_status.startup_fail = msg_data_a->elcon_charger_status.startup_fail;
                    can_data.elcon_charger_status.communication_fail = msg_data_a->elcon_charger_status.communication_fail;
                    can_data.elcon_charger_status.stale = 0;
                    can_data.elcon_charger_status.last_rx = sched.os_ticks;
                    break;
                case ID_DAQ_COMMAND_PRECHARGE:
                    can_data.daq_command_PRECHARGE.daq_command = msg_data_a->daq_command_PRECHARGE.daq_command;
                    daq_command_PRECHARGE_CALLBACK(&msg_header);
                    break;
                default:
                    __asm__("nop");
            }
        }
        /* END AUTO CASES */
    }

    /* BEGIN AUTO STALE CHECKS */
    CHECK_STALE(can_data.elcon_charger_status.stale,
                sched.os_ticks, can_data.elcon_charger_status.last_rx,
                UP_ELCON_CHARGER_STATUS);
    /* END AUTO STALE CHECKS */
}

bool initCANFilter()
{
    CAN1->MCR |= CAN_MCR_INRQ;                // Enter back into INIT state (required for changing scale)
#ifdef CAN2
    CAN2->MCR |= CAN_MCR_INRQ; 
#endif /* CAN2 */                
    uint32_t timeout = 0;
    while( 
        !(CAN1->MSR & CAN_MSR_INAK)
#ifdef CAN2
           && !(CAN2->MSR & CAN_MSR_INAK)
#endif /* CAN2 */
           && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
         return false;

    // Allow all messages from both busses
    CAN1->FM1R |= 0x00000000;                 // Set banks 0-27 to mask mode
    CAN1->FA1R |= (1 << 0);    // configure bank 0
    CAN1->sFilterRegister[0].FR1 = 0;
    CAN1->sFilterRegister[0].FR2 = 0;
    
    CAN1->FA1R |= (1 << 17);    // configure bank 16 for CAN2
    CAN1->sFilterRegister[17].FR1 = 0;
    CAN1->sFilterRegister[17].FR2 = 0;

    CAN2->FA1R |= (1 << 20);    // configure bank 16 for CAN2
    CAN2->sFilterRegister[20].FR1 = 0;
    CAN2->sFilterRegister[20].FR2 = 0;

    /* BEGIN AUTO FILTER */
    /* END AUTO FILTER */

    CAN1->FMR  &= ~CAN_FMR_FINIT;       // Enable Filters (exit filter init mode)
    CAN1->MCR &= ~CAN_MCR_INRQ;         // Enter back into NORMAL mode
#ifdef CAN2
    CAN2->FMR  &= ~CAN_FMR_FINIT;       // Enable Filters (exit filter init mode)
    CAN2->MCR &= ~CAN_MCR_INRQ;         // Enter back into NORMAL mode
#endif /* CAN2 */

    while(
        (CAN1->MSR & CAN_MSR_INAK)
#ifdef CAN2 
            && (CAN2->MSR & CAN_MSR_INAK)
#endif/* CAN2 */
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
