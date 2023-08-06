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

extern uint32_t APB1ClockRateHz;

bool PHAL_initCAN(CAN_TypeDef* bus, bool test_mode)
{
    uint32_t timeout = 0;

    // Enable CAN Clock 
    if (bus == CAN1)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_CAN1EN;
    }
    #ifdef CAN2
    else if (bus == CAN2)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_CAN2EN;
    } 
    #endif /* CAN2 */
    else
    {
        return false;
    }

    // Leave SLEEP state
    bus->MCR &= ~CAN_MCR_SLEEP; 
    // Enter INIT state
    bus->MCR |= CAN_MCR_INRQ;
    while((bus->MSR & CAN_MSR_SLAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
        return false;
    timeout = 0;

    bus->MCR |= CAN_MCR_INRQ;
    while(!(bus->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
        return false;
    timeout = 0;

    // Bit timing recovered from http://www.bittiming.can-wiki.info/
    switch (APB1ClockRateHz)
    {
        case 16000000:
            bus->BTR = PHAL_CAN_16MHz_500k;
            break;
        case 20000000:
            bus->BTR = PHAL_CAN_20MHz_500k;
            break;
        case 40000000:
            bus->BTR = PHAL_CAN_40MHz_500k;
            break;
        case 80000000:
            bus->BTR = PHAL_CAN_80MHz_500k;
            break;
        default:
            return false;
    }
    // Keep the bus active
    bus->MCR |= CAN_MCR_ABOM;

    // Loopback mode in testing mode
    if (test_mode)
    {
        bus->BTR |= CAN_BTR_LBKM;
    }
    
    // Setup filters for all IDs
    bus->FMR  |= CAN_FMR_FINIT;              // Enter init mode for filter banks
    bus->FM1R &= ~(CAN_FM1R_FBM0_Msk);       // Set bank 0 to mask mode
    bus->FS1R &= ~(1 << CAN_FS1R_FSC0_Pos);  // Set bank 0 to 16bit mode
    bus->FA1R |= (1 << CAN_FA1R_FACT0_Pos);  // Activate bank 0
    bus->sFilterRegister[0].FR1 = 0;         // Set mask to 0
    bus->sFilterRegister[0].FR2 = 0;
    bus->FMR  &= ~CAN_FMR_FINIT;             // Enable Filters

    // Enable FIFO0/1 RX message pending interrupt
    bus->IER |= CAN_IER_FMPIE0;
    bus->IER |= CAN_IER_FMPIE1;

    // Enter NORMAL mode
    bus->MCR &= ~CAN_MCR_INRQ;
    while((bus->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;

    return timeout != PHAL_CAN_INIT_TIMEOUT;
}

bool PHAL_deinitCAN(CAN_TypeDef* bus)
{
    if (bus == CAN1)
    {
        RCC->APB1RSTR1 |= RCC_APB1RSTR1_CAN1RST;
    }
    #ifdef CAN2
    else if(bus == CAN2)
    {
        RCC->APB1RSTR1 |= RCC_APB1RSTR1_CAN2RST;
    }
    #endif /* CAN2 */
    else return false;
    return true;
}

bool PHAL_txCANMessage(CanMsgTypeDef_t* msg)
{
    uint8_t txMbox = 0;
    uint32_t timeout = 0;
    uint32_t txOkay = 0;

    if (msg->Bus->TSR & CAN_TSR_TME0)
    {
        txMbox = 0;
        txOkay = CAN_TSR_TXOK0;
    }
    else if (msg->Bus->TSR & CAN_TSR_TME1)
    {
        txMbox = 1;
        txOkay = CAN_TSR_TXOK1;
    }
    else if (msg->Bus->TSR & CAN_TSR_TME2)
    {
        txMbox = 2;
        txOkay = CAN_TSR_TXOK2;
    }
    else   
        return false;   // Unable to find Mailbox

    if (msg->IDE == 0)
    {
        msg->Bus->sTxMailBox[txMbox].TIR  = (msg->StdId << CAN_TI0R_STID_Pos);  // Standard ID
    }
    else
    {
        msg->Bus->sTxMailBox[txMbox].TIR  = (msg->ExtId << CAN_TI0R_EXID_Pos) | 4;  // Extended ID
    }
    msg->Bus->sTxMailBox[txMbox].TDTR = (msg->DLC << CAN_TDT0R_DLC_Pos);    // Data Length
    msg->Bus->sTxMailBox[txMbox].TDLR = ((uint32_t) msg->Data[3] << 24) |
                                        ((uint32_t) msg->Data[2] << 16) |
                                        ((uint32_t) msg->Data[1] << 8)  |
                                        ((uint32_t) msg->Data[0]);
    msg->Bus->sTxMailBox[txMbox].TDHR = ((uint32_t) msg->Data[7] << 24) |
                                        ((uint32_t) msg->Data[6] << 16) |
                                        ((uint32_t) msg->Data[5] << 8)  |
                                        ((uint32_t) msg->Data[4]);
    
    msg->Bus->sTxMailBox[txMbox].TIR |= (0b1 << CAN_TI0R_TXRQ_Pos);         // Request TX

    while(!(msg->Bus->TSR & txOkay) && ++timeout < PHAL_CAN_TX_TIMEOUT)      // Wait for message to be sent within specified timeout
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

#ifdef STM32L496xx
void  __attribute__((weak)) CAN2_RX0_IRQHandler()
{
    // Implement for RX Mailbox 0 Handler
}

void  __attribute__((weak)) CAN2_RX1_IRQHandler()
{
    // Implement for RX Mailbox 1 Handler
}
#endif