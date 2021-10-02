/**
 * @file hal_can_f4.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Basic CAN Peripheral HAL library for setting up CAN peripheral and sending messages
 * @version 0.1
 * @date 2021-02-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "common/phal_L4/can/can.h"

bool PHAL_initCAN(bool test_mode)
{
    uint32_t timeout = 0;

    // Enable CAN Clock 
    RCC->APB1ENR1 |= RCC_APB1ENR1_CAN1EN;
    asm("nop"); // Ensure that clock is enabled

    // Leave SLEEP state
    CAN1->MCR &= ~CAN_MCR_SLEEP; 
    while((CAN1->MSR & CAN_MSR_SLAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
        return false;
    timeout = 0;

    // Enter INIT state
    CAN1->MCR |= CAN_MCR_INRQ;
    while(!(CAN1->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
        return false;
    timeout = 0;

    // Bit timing recovered from http://www.bittiming.can-wiki.info/
    CAN1->BTR = 0x00050000;
    
    // Keep the bus active
    CAN1->MCR |= CAN_MCR_ABOM;

    // Loopback mode in testing mode
    if (test_mode)
    {
        CAN1->BTR |= CAN_BTR_LBKM;
    }
    
    // Setup filters for all IDs
    CAN1->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    CAN1->FM1R &= ~(CAN_FM1R_FBM0_Msk);       // Set bank 0 to mask mode
    CAN1->FS1R &= ~(1 << CAN_FS1R_FSC0_Pos);  // Set bank 0 to 16bit mode
    CAN1->FA1R |= (1 << CAN_FA1R_FACT0_Pos);  // Activate bank 0
    CAN1->sFilterRegister[0].FR1 = 0;         // Set mask to 0
    CAN1->sFilterRegister[0].FR2 = 0;
    CAN1->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters

    // Enable FIFO0/1 RX message pending interrupt
    CAN1->IER |= CAN_IER_FMPIE0;
    CAN1->IER |= CAN_IER_FMPIE1;

    // Enter NORMAL mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while((CAN1->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;

    return timeout != PHAL_CAN_INIT_TIMEOUT;
}

bool PHAL_deinitCAN()
{
    RCC->APB1RSTR1 |= RCC_APB1RSTR1_CAN1RST;
    return true;
}

bool PHAL_txCANMessage(CanMsgTypeDef_t* msg)
{
    uint8_t txMbox = 0;
    uint32_t timeout = 0;
    uint32_t txOkay = 0;

    if (CAN1->TSR & CAN_TSR_TME0)
    {
        txMbox = 0;
        txOkay = CAN_TSR_TXOK0;
    }
    else if (CAN1->TSR & CAN_TSR_TME1)
    {
        txMbox = 1;
        txOkay = CAN_TSR_TXOK1;
    }
    else if (CAN1->TSR & CAN_TSR_TME2)
    {
        txMbox = 2;
        txOkay = CAN_TSR_TXOK2;
    }
    else   
        return false;   // Unable to find Mailbox

    if (msg->IDE == 0)
    {
        CAN1->sTxMailBox[txMbox].TIR  = (msg->StdId << CAN_TI0R_STID_Pos);  // Standard ID
    }
    else
    {
        CAN1->sTxMailBox[txMbox].TIR  = (msg->ExtId << CAN_TI0R_EXID_Pos) | 4;  // Extended ID
    }
    CAN1->sTxMailBox[txMbox].TDTR = (msg->DLC << CAN_TDT0R_DLC_Pos);    // Data Length
    CAN1->sTxMailBox[txMbox].TDLR = ((uint32_t) msg->Data[3] << 24) |
                                    ((uint32_t) msg->Data[2] << 16) |
                                    ((uint32_t) msg->Data[1] << 8)  |
                                    ((uint32_t) msg->Data[0]);
    CAN1->sTxMailBox[txMbox].TDHR = ((uint32_t) msg->Data[7] << 24) |
                                    ((uint32_t) msg->Data[6] << 16) |
                                    ((uint32_t) msg->Data[5] << 8)  |
                                    ((uint32_t) msg->Data[4]);
    
    CAN1->sTxMailBox[txMbox].TIR |= (0b1 << CAN_TI0R_TXRQ_Pos);         // Request TX

    while(!(CAN1->TSR & txOkay) && ++timeout < PHAL_CAN_TX_TIMEOUT)      // Wait for message to be sent within specified timeout
        ;

    return timeout != PHAL_CAN_TX_TIMEOUT;
}

void  __attribute__((weak)) CAN1_RX0_IRQHandler()
{
    // Implement for RX Mailbox 0 Handler
}

void  __attribute__((weak)) CAN1_RX1_IRQHandler()
{
    // Implement for RX Mailbox 1 Handler
}