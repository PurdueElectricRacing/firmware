/**
 * @file can.c
 * @brief F4 bxCAN initialization, filters, and interrupt-driven frame transport.
 * @copyright Copyright (c) 2021
 */
#include "common/phal_F4/can/can.h"

extern uint32_t APB1ClockRateHz;

static bool valid_bus(const CAN_TypeDef *bus) {
    if (bus == CAN1)
        return true;
#ifdef CAN2
    if (bus == CAN2)
        return true;
#endif
    return false;
}

static bool wait_init(CAN_TypeDef *bus, bool enabled) {
    for (uint32_t timeout = 0; timeout < PHAL_CAN_INIT_TIMEOUT; ++timeout) {
        if (((bus->MSR & CAN_MSR_INAK) != 0) == enabled)
            return true;
    }
    return false;
}

bool PHAL_CAN_init(CAN_TypeDef *bus, PHAL_FDCAN_BaudRate_t bit_rate) {
    if (!valid_bus(bus))
        return false;
    // CAN2 depends on CAN1's clock, including for the shared filter registers.
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
#ifdef CAN2
    if (bus == CAN2)
        RCC->APB1ENR |= RCC_APB1ENR_CAN2EN;
#endif
    bus->MCR &= ~CAN_MCR_SLEEP;
    bus->MCR |= CAN_MCR_INRQ;
    if (!wait_init(bus, true))
        return false;

    // Bit timing recovered from http://www.bittiming.can-wiki.info/
    if (bit_rate == 1'000'000) {
        switch (APB1ClockRateHz) {
            case 16'000'000:
                bus->BTR = PHAL_CAN_16MHz_1M;
                break;
            case 24'000'000:
                bus->BTR = PHAL_CAN_24MHz_1M;
                break;
            case 36'000'000:
                bus->BTR = PHAL_CAN_36MHz_1M;
                break;
            case 42'000'000:
                bus->BTR = PHAL_CAN_42MHz_1M;
                break;
            default:
                return false;
        }
    } else if (bit_rate == 500'000) {
        switch (APB1ClockRateHz) {
            case 16'000'000:
                bus->BTR = PHAL_CAN_16MHz_500k;
                break;
            case 24'000'000:
                bus->BTR = PHAL_CAN_24MHz_500k;
                break;
            case 36'000'000:
                bus->BTR = PHAL_CAN_36MHz_500k;
                break;
            case 42'000'000:
                bus->BTR = PHAL_CAN_42MHz_500k;
                break;
            default:
                return false;
        }
    } else if (bit_rate == 250'000) {
        switch (APB1ClockRateHz) {
            case 16'000'000:
                bus->BTR = PHAL_CAN_16MHz_250k;
                break;
            case 24'000'000:
                bus->BTR = PHAL_CAN_24MHz_250k;
                break;
            case 36'000'000:
                bus->BTR = PHAL_CAN_36MHz_250k;
                break;
            default:
                return false;
        }

    } else {
        return false;
    }

    bus->MCR |= CAN_MCR_ABOM;
    if (!PHAL_CAN_setFilters(bus, 0, 0, 0, 0))
        return false;
    bus->IER &= ~CAN_IER_FMPIE1;
    bus->IER |= CAN_IER_FMPIE0 | CAN_IER_TMEIE;
    bus->MCR &= ~CAN_MCR_INRQ;
    return wait_init(bus, false);
}

bool PHAL_CAN_setFilters(CAN_TypeDef *bus,
                         const uint32_t *sid_list,
                         uint32_t num_sid,
                         const uint32_t *xid_list,
                         uint32_t num_xid) {
    if (!valid_bus(bus) || num_sid > PHAL_CAN_MAX_NUM_SID_FILTER
        || num_xid > PHAL_CAN_MAX_NUM_XID_FILTER || num_sid + num_xid > 28 || (num_sid && !sid_list)
        || (num_xid && !xid_list))
        return false;
    for (uint32_t i = 0; i < num_sid; ++i)
        if (sid_list[i] > 0x7FFU)
            return false;
    for (uint32_t i = 0; i < num_xid; ++i)
        if (xid_list[i] > 0x1FFFFFFFU)
            return false;

    // Preserve CAN1's init state when called during initialization or by CAN2.
    bool was_init = (CAN1->MCR & CAN_MCR_INRQ) != 0;
    CAN1->MCR &= ~CAN_MCR_SLEEP;
    CAN1->MCR |= CAN_MCR_INRQ;
    if (!wait_init(CAN1, true)) {
        if (!was_init)
            CAN1->MCR &= ~CAN_MCR_INRQ;
        return false;
    }
    uint32_t base = 0;
#ifdef CAN2
    if (bus == CAN2)
        base = 14;
#endif
    uint32_t owned = 0x3FFFU << base;
    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FMR = (CAN1->FMR & ~CAN_FMR_CAN2SB) | (14U << CAN_FMR_CAN2SB_Pos);
    CAN1->FA1R &= ~owned;
    CAN1->FFA1R &= ~owned; // Route all owned banks to FIFO0.
    CAN1->FS1R |= owned;   // Two 32-bit filter words per bank.
    CAN1->FM1R |= owned;   // ID-list mode.

    uint32_t count = num_sid + num_xid;
    if (count == 0) {
        CAN1->FM1R &= ~(1U << base); // Mask mode: accept all.
        CAN1->sFilterRegister[base].FR1 = 0;
        CAN1->sFilterRegister[base].FR2 = 0;
        CAN1->FA1R |= 1U << base;
    } else {
        for (uint32_t i = 0; i < count; i += 2) {
            uint32_t words[2];
            for (uint32_t j = 0; j < 2; ++j) {
                // Duplicate the last ID for an odd count, avoiding an extra match.
                uint32_t index = (i + j < count) ? i + j : i;
                words[j] =
                    index < num_sid ? sid_list[index] << 21 : (xid_list[index - num_sid] << 3) | 4U;
            }
            uint32_t bank                   = base + i / 2;
            CAN1->sFilterRegister[bank].FR1 = words[0];
            CAN1->sFilterRegister[bank].FR2 = words[1];
            CAN1->FA1R |= 1U << bank;
        }
    }
    CAN1->FMR &= ~CAN_FMR_FINIT;
    if (was_init)
        return true;
    CAN1->MCR &= ~CAN_MCR_INRQ;
    return wait_init(CAN1, false);
}

bool PHAL_CAN_send(CanMsgTypeDef_t *msg) {
    if (!msg || !valid_bus(msg->Bus) || msg->DLC > 8
        || (msg->IDE ? msg->ExtId > 0x1FFFFFFFU : msg->StdId > 0x7FFU))
        return false;
    uint32_t tsr = msg->Bus->TSR;
    uint32_t txMbox;
    if (tsr & CAN_TSR_TME0)
        txMbox = 0;
    else if (tsr & CAN_TSR_TME1)
        txMbox = 1;
    else if (tsr & CAN_TSR_TME2)
        txMbox = 2;
    else
        return false;

    if (!msg->IDE) {
        msg->Bus->sTxMailBox[txMbox].TIR =
            ((uint32_t)msg->StdId << CAN_TI0R_STID_Pos); // Standard ID
    } else {
        msg->Bus->sTxMailBox[txMbox].TIR = (msg->ExtId << CAN_TI0R_EXID_Pos) | 4; // Extended ID
    }
    msg->Bus->sTxMailBox[txMbox].TDTR = (msg->DLC << CAN_TDT0R_DLC_Pos); // Data Length
    msg->Bus->sTxMailBox[txMbox].TDLR = ((uint32_t)msg->Data[3] << 24)
        | ((uint32_t)msg->Data[2] << 16) | ((uint32_t)msg->Data[1] << 8) | ((uint32_t)msg->Data[0]);
    msg->Bus->sTxMailBox[txMbox].TDHR = ((uint32_t)msg->Data[7] << 24)
        | ((uint32_t)msg->Data[6] << 16) | ((uint32_t)msg->Data[5] << 8) | ((uint32_t)msg->Data[4]);

    msg->Bus->sTxMailBox[txMbox].TIR |= (0b1 << CAN_TI0R_TXRQ_Pos); // Request TX

    msg->Bus->IER |= CAN_IER_TMEIE;
    return true;
}

bool PHAL_CAN_txFifoFree(const CAN_TypeDef *bus) {
    return valid_bus(bus) && (bus->TSR & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2));
}

static void rx_irq(CAN_TypeDef *bus) {
    while (bus->RF0R & CAN_RF0R_FMP0) {
        CanMsgTypeDef_t msg = {.Bus = bus};
        uint32_t rir        = bus->sFIFOMailBox[0].RIR;
        msg.IDE             = (rir & CAN_RI0R_IDE) != 0;
        if (msg.IDE)
            msg.ExtId = rir >> CAN_RI0R_EXID_Pos;
        else
            msg.StdId = rir >> CAN_RI0R_STID_Pos;
        msg.DLC = (bus->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC) >> CAN_RDT0R_DLC_Pos;
        if (msg.DLC > 8)
            msg.DLC = 8;
        uint32_t low  = bus->sFIFOMailBox[0].RDLR;
        uint32_t high = bus->sFIFOMailBox[0].RDHR;
        for (uint32_t i = 0; i < 4; ++i) {
            msg.Data[i]     = low >> (8 * i);
            msg.Data[i + 4] = high >> (8 * i);
        }
        bus->RF0R = CAN_RF0R_RFOM0;
        PHAL_CAN_rxCallback(&msg);
    }
}

static void tx_irq(CAN_TypeDef *bus) {
    bus->IER &= ~CAN_IER_TMEIE;
    uint32_t tsr         = bus->TSR;
    bus->TSR             = tsr & (CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2);
    const uint32_t empty = CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2;
    if ((tsr & empty) != empty)
        bus->IER |= CAN_IER_TMEIE;
    PHAL_CAN_txCallback(bus);
}

void CAN1_RX0_IRQHandler(void) {
    rx_irq(CAN1);
}

void CAN1_TX_IRQHandler(void) {
    tx_irq(CAN1);
}
#ifdef CAN2
void CAN2_RX0_IRQHandler(void) {
    rx_irq(CAN2);
}

void CAN2_TX_IRQHandler(void) {
    tx_irq(CAN2);
}
#endif

[[gnu::weak]]
void PHAL_CAN_txCallback(CAN_TypeDef *can) {
    (void)can;
}

[[gnu::weak]]
void PHAL_CAN_rxCallback(CanMsgTypeDef_t *msg) {
    (void)msg;
}
