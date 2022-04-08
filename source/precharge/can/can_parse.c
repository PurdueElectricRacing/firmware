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
        if (msg_header.Bus == CAN2)
        {
            switch(msg_header.ExtId)
            {
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
                default:
                    __asm__("nop");
            }
        }
        else if (msg_header.Bus == CAN1)
        {
            switch(msg_header.ExtId)
            {
                default:
                    __asm__("nop");
            }
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
           && !(CAN2->MSR & CAN_MSR_INAK)
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
    CAN2->FA1R |= (1 << 0);    // configure bank 0
    CAN2->sFilterRegister[0].FR1 = (ID_SOC_CELLS << 3) | 4;
    CAN2->sFilterRegister[0].FR2 = (ID_VOLTS_CELLS << 3) | 4;
    CAN2->FA1R |= (1 << 1);    // configure bank 1
    CAN2->sFilterRegister[1].FR1 = (ID_PACK_INFO << 3) | 4;
    CAN2->sFilterRegister[1].FR2 = (ID_TEMPS_CELLS << 3) | 4;
    CAN2->FA1R |= (1 << 2);    // configure bank 2
    CAN2->sFilterRegister[2].FR1 = (ID_CELL_INFO << 3) | 4;
    CAN2->sFilterRegister[2].FR2 = (ID_POWER_LIM << 3) | 4;
    /* END AUTO FILTER */

    CAN1->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters (exit filter init mode)

    // Enter back into NORMAL mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
#ifdef CAN2
    CAN2->MCR &= ~CAN_MCR_INRQ;
#endif /* CAN2 */
    while((CAN1->MSR & CAN_MSR_INAK)
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
