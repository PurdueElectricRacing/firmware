/**
 * @file fdcan_priv.h
 * @brief G4 FDCAN private/register level implementation
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#include "common/phal_G4/fdcan/fdcan.h"
#include "common/phal_G4/fdcan/fdcan_priv.h"
 
uint32_t PHAL_FDCAN_priv_ramBase(FDCAN_GlobalTypeDef *fdcan) {
    if (fdcan == FDCAN1) {
        return SRAMCAN_BASE;
    } else if (fdcan == FDCAN2) {
        return SRAMCAN_BASE + FDCAN_PRIV_SRAMCAN_SIZE;
    } else { // fdcan == FDCAN3
        return SRAMCAN_BASE + 2U * FDCAN_PRIV_SRAMCAN_SIZE;
    }
}
 
void PHAL_FDCAN_priv_enableClock(void) {
    // RCC = Reset and Clock Control peripheral.
    // CCIPR = Peripheral Independent Clock Configuration Register
    // - select which clock source feeds peripherals
    // FDCANSEL = FDCAN clock source Select field within CCIPR
    // FDCANSEL_Msk = bitmask covering the full FDCANSEL field
    // - (so it can be cleared before writing a new value into it)
    RCC->CCIPR &= ~RCC_CCIPR_FDCANSEL_Msk;

    // Set FDCAN clock source to PCLK1
    RCC->CCIPR |= FDCAN_PRIV_RCC_FDCANCLKSOURCE_PCLK1;

    // APB1ENR1 = Advanced Peripheral Bus 1 (APB1) (peripheral clock) Enable
    // Register 1 - each bit gates the clock to one APB1 peripheral
    // FDCANEN = FDCAN clock Enable bit within APB1ENR1
    // - until this is set the peripheral's registers aren't clocked/accessible
    RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
}
 
void PHAL_FDCAN_priv_enterConfig(FDCAN_GlobalTypeDef *fdcan) {
    // CCCR = Communication Controller (CC) Control Register
    // - main mode/state register for the FDCAN core
    // CSR = Clock Stop Request bit
    // - clear to ensure we are not requesting clock stop before we configure it
    fdcan->CCCR &= ~FDCAN_CCCR_CSR;

    // CSA = Clock Stop Acknowledge bit
    // - hardware sets this once it has entered clock-stop state in response to CSR
    // - wait until the peripheral confirms it has left clock-stop state (CSA low)
    // - then it's ready to accept further configuration
    while (fdcan->CCCR & FDCAN_CCCR_CSA) {
        __asm__("nop");
    }

    // INIT = Initialization bit
    // - must be set to 1 before configuring CCCR
    // - now RAM configuration is allowed to change
    fdcan->CCCR |= FDCAN_CCCR_INIT;

    // Wait until hardware reflects INIT=1 back
    while (!(fdcan->CCCR & FDCAN_CCCR_INIT)) {
        __asm__("nop");
    }

    // CCE = Configuration Change Enable bit
    // - CCE unlocks the protected configuration registers
    //   (NBTP, TXBC, RXGFC) for writing
    fdcan->CCCR |= FDCAN_CCCR_CCE;
}
 
void PHAL_FDCAN_priv_exitConfig(FDCAN_GlobalTypeDef *fdcan) {
    // Clearing INIT (Initialization bit) tells the core to leave
    // initialization/configuration mode and resume normal bus operation
    fdcan->CCCR &= ~FDCAN_CCCR_INIT;

    // Busy-wait until hardware confirms INIT is 0
    while (fdcan->CCCR & FDCAN_CCCR_INIT) {
        __asm__("nop");
    }
}

void PHAL_FDCAN_priv_controlConfig(FDCAN_GlobalTypeDef *fdcan) {
    // FDOE = FD (Flexible Data-rate) Operation Enable bit
    // - cleared: forces classic (non-FD) CAN only
    // BRSE = Bit Rate Switch Enable bit
    // - cleared: only matters for FD frames, cleared b/c FD is disabled
    // MON = Bus Monitoring mode bit
    // - cleared: otherwise puts the core in a listen-only mode (no ACK/TX/etc)
    // ASM = Restricted Operation Mode bit
    // - cleared: otherwise puts core in  a listen-and-limited-ACK mode
    // TEST = Test Mode Enable bit
    // - cleared: not configuring TEST register and not using loopback mode
    fdcan->CCCR &= ~(FDCAN_CCCR_FDOE | FDCAN_CCCR_BRSE | FDCAN_CCCR_MON | FDCAN_CCCR_ASM | FDCAN_CCCR_TEST);

    // PXHD = Protocol Exception Handling Disable bit
    // - setting this to 1 disables "protocol exception" bus-off-avoidance handling
    // - now: unusual bit sequences are treated as ordinary form errors instead
    fdcan->CCCR |= FDCAN_CCCR_PXHD;

    // DAR = Disable Automatic Retransmission bit
    // - clearing this bit means automatic retransmission is enabled
    // - now: a failed/aborted frame will be retried automatically by hardware
    fdcan->CCCR &= ~FDCAN_CCCR_DAR;

    // TXP = Transmit Pause bit
    // - set: the core adds very short delay after each tx before starting the next
    //   to give other nodes a fairer chance at the bus
    fdcan->CCCR |= FDCAN_CCCR_TXP;

    // TEST = the FDCAN Test register
    // LBCK = Loop Back mode bit within TEST
    // - not set: does not internally routes TX back to RX inside the peripheral
    // - now: TX is sent to the bus and RX only sees what is actually on the bus
    fdcan->TEST &= ~FDCAN_TEST_LBCK; // disable loopback
}

void PHAL_FDCAN_priv_setTransmitFifoQueueModeToFifo(FDCAN_GlobalTypeDef *fdcan) {
    // TXBC = TX Buffer Configuration register
    // - controls how the TX buffer area is used
    // TFQM = Tx FIFO/Queue Mode bit
    // - clear: selects FIFO ordering (tx in the order they were queued)
    fdcan->TXBC &= ~FDCAN_TXBC_TFQM;
}

void PHAL_FDCAN_setInteruptLines(FDCAN_GlobalTypeDef *fdcan) {
    fdcan->ILS     = FDCAN_ILS_SMSG;
    fdcan->ILE     = FDCAN_ILE_EINT0 | FDCAN_ILE_EINT1;
    fdcan->IR      = FDCAN_IR_RF0N | FDCAN_IR_TC; // clear any stale flags
    fdcan->IE     |= FDCAN_IE_RF0NE | FDCAN_IE_TCE;
    fdcan->TXBTIE  = 0xFFFFFFFFU; // TX complete interrupt for every TX buffer
}

void PHAL_FDCAN_priv_writeFilterAction(FDCAN_GlobalTypeDef *fdcan, PHAL_FDCAN_FilterAction_t action) {
    fdcan->RXGFC = (action << FDCAN_RXGFC_ANFS_Pos)
        | (action << FDCAN_RXGFC_ANFE_Pos)
        | (action << FDCAN_RXGFC_RRFS_Pos)
        | (action << FDCAN_RXGFC_RRFE_Pos);
}
 
// Builds nominal bit timing & prescaler register for a
// 16-time-quantum nominal bit time (~87.5% sample point) at the given BRP
// (BRP=2 @ 500k, BRP=1 @ 1M)
static uint32_t fdcan_buildNBTP16TQ(uint32_t brp) {
    const uint32_t seg1 = 13, seg2 = 2, sjw = 2;
    return ((brp - 1U) << FDCAN_NBTP_NBRP_Pos) | ((seg1 - 1U) << FDCAN_NBTP_NTSEG1_Pos)
        | ((seg2 - 1U) << FDCAN_NBTP_NTSEG2_Pos) | ((sjw - 1U) << FDCAN_NBTP_NSJW_Pos);
}

uint32_t PHAL_FDCAN_priv_getNBTP(PHAL_FDCAN_BaudRate_t bit_rate) {
    switch (bit_rate) {
        case FDCAN_BAUD_500K:
            return fdcan_buildNBTP16TQ(FDCAN_PRIV_KER_CLK_HZ / (500000U * 16U));
        case FDCAN_BAUD_1M:
            return fdcan_buildNBTP16TQ(FDCAN_PRIV_KER_CLK_HZ / (1000000U * 16U));
    }
    __builtin_unreachable();
}
 
void PHAL_FDCAN_priv_writeStandardFilters(FDCAN_GlobalTypeDef *fdcan, uint32_t *sid_list, uint32_t num_sid) {
    volatile uint32_t *ram = (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan)
                                 + FDCAN_PRIV_SRAMCAN_FLSSA);
 
    // Set up each filter element
    for (uint32_t i = 0; i < num_sid; i++) {
        uint32_t sid = sid_list[i] & 0x7FFU;
        // SFT=10 (classic mask), SFEC=001 (store to FIFO0), SFID1=sid, SFID2=mask (all 1s -> exact match)
        ram[i] = (2U << 30) | (1U << 27) | (sid << 16) | 0x7FFU;
    }
 
    // Set the number of standard filters
    fdcan->RXGFC &= ~FDCAN_RXGFC_LSS_Msk;
    fdcan->RXGFC |= (num_sid << FDCAN_RXGFC_LSS_Pos);
}
 
void PHAL_FDCAN_priv_writeExtendedFilters(FDCAN_GlobalTypeDef *fdcan, uint32_t *xid_list, uint32_t num_xid) {
    volatile uint32_t *ram = (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan)
                                 + FDCAN_PRIV_SRAMCAN_FLESA);
 
    // Set up each filter element
    for (uint32_t i = 0; i < num_xid; i++) {
        uint32_t xid = xid_list[i] & 0x1FFFFFFFU;
        ram[i * 2 + 0] = (1U << 29) | xid;         // EFEC=001 (store to FIFO0), EFID1=id
        ram[i * 2 + 1] = (2U << 30) | 0x1FFFFFFFU; // EFT=10 (classic mask), mask=all 1s -> exact match
    }
 
    // Set the number of extended filters
    fdcan->RXGFC &= ~FDCAN_RXGFC_LSE_Msk;
    fdcan->RXGFC |= (num_xid << FDCAN_RXGFC_LSE_Pos);
    fdcan->XIDAM = 0x1FFFFFFFU;
}
 
void PHAL_FDCAN_priv_writeTxElement(FDCAN_GlobalTypeDef *fdcan, CanMsgTypeDef_t *msg) {
    // Get the index of the next free TX FIFO slot
    uint32_t put = (fdcan->TXFQS & FDCAN_TXFQS_TFQPI_Msk) >> FDCAN_TXFQS_TFQPI_Pos;
    volatile uint32_t *tx = (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan)
                                + FDCAN_PRIV_SRAMCAN_TFQSA
                                + (put * FDCAN_PRIV_SRAMCAN_TFQ_SIZE));
 
    // Build the TX element header and payload
    uint32_t dlc = msg->DLC > 8U ? 8U : msg->DLC;
 
    tx[0] = msg->IsExtendedId ? ((msg->ExtId & 0x1FFFFFFFU) | (1U << 30)) // extended ID, IDE=1
                              : ((uint32_t)(msg->StdId & 0x7FFU) << 18);  // standard ID, IDE=0
    tx[1] = dlc << 16; // classic CAN: BRS=0, FDF=0, EFC=0, MM=0, RTR=0
 
    // Copy payload bytes into two 32-bit words
    uint32_t d0 = 0, d1 = 0;
    for (uint32_t i = 0; i < dlc && i < 4U; i++) {
        d0 |= (uint32_t)msg->Data[i] << (8U * i);
    }
    for (uint32_t i = 4; i < dlc; i++) {
        d1 |= (uint32_t)msg->Data[i] << (8U * (i - 4U));
    }
    tx[2] = d0;
    tx[3] = d1;
 
    // Mark the element as ready to transmit
    fdcan->TXBAR = (1U << put);
}
 
bool PHAL_FDCAN_priv_readRxElement(FDCAN_GlobalTypeDef *fdcan, CanMsgTypeDef_t *msg) {
    uint32_t f0s = fdcan->RXF0S;
    if ((f0s & FDCAN_RXF0S_F0FL_Msk) == 0) {
        return false; // FIFO0 empty
    }
 
    // Get the index of the oldest element in RX FIFO0
    uint32_t get = (f0s & FDCAN_RXF0S_F0GI_Msk) >> FDCAN_RXF0S_F0GI_Pos;
    volatile uint32_t *rx = (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan)
                                + FDCAN_PRIV_SRAMCAN_RF0SA
                                + (get * FDCAN_PRIV_SRAMCAN_RF0_SIZE));
 
    // Decode the RX element header into a CanMsgTypeDef_t
    uint32_t w0 = rx[0];
    uint32_t w1 = rx[1];
 
    *msg     = (CanMsgTypeDef_t) {0};
    msg->Bus = fdcan;
    if (w0 & (1U << 30)) {
        msg->IsExtendedId = true;
        msg->ExtId        = w0 & 0x1FFFFFFFU;
    } else {
        msg->IsExtendedId = false;
        msg->StdId        = (uint16_t)((w0 >> 18) & 0x7FFU);
    }
 
    // Decode DLC
    uint8_t dlc = ((w1 >> 16) & 0xF);
    uint8_t len = dlc > 8U ? 8U : dlc; // classic CAN max payload is 8
    if (len > sizeof(msg->Data)) {
        len = sizeof(msg->Data);
    }
    msg->DLC = len;
 
    // Copy payload bytes
    const uint8_t* payload = (const uint8_t *)&rx[2];
    for (uint8_t i = 0; i < len; i++) {
        msg->Data[i] = payload[i];
    }
 
    // Pop the element we just read
    fdcan->RXF0A = get;

    return true;
}

bool PHAL_FDCAN_priv_readTxFifoQueueStatusFullFlag(FDCAN_GlobalTypeDef *fdcan) {
    // TXFQS = Tx FIFO/Queue Status register
    // TFQF = Tx FIFO/Queue Full flag
    // - 1 when every TX FIFO/queue element is currently occupied by a pending/queued frame
    return (fdcan->TXFQS & FDCAN_TXFQS_TFQF) != 0;
}

bool PHAL_FDCAN_priv_readReceiveFifo0NewMessageInterruptFlag(FDCAN_GlobalTypeDef *fdcan) {
    // IR = Interrupt Register
    // - latched interrupt flags
    // RF0N = Receive FIFO 0 New message flag
    // - set by hardware whenever a new frame has been stored into RX FIFO 0
    return (fdcan->IR & FDCAN_IR_RF0N) != 0;
}

void PHAL_FDCAN_priv_clearReceiveFifo0NewMessageInterruptFlag(FDCAN_GlobalTypeDef *fdcan) {
    // IR bits are write-1-to-clear
    // - writing RF0N (Receive FIFO 0 New message flag) back to IR clears only that flag
    fdcan->IR = FDCAN_IR_RF0N;
}

bool PHAL_FDCAN_priv_readTransmitCompleteInterruptFlag(FDCAN_GlobalTypeDef *fdcan) {
    // TC = Transmission Completed flag within IR
    // - set by hardware once a queued TX element has finished transmitting successfully
    return (fdcan->IR & FDCAN_IR_TC) != 0;
}

void PHAL_FDCAN_priv_clearTransmitCompleteInterruptFlag(FDCAN_GlobalTypeDef *fdcan) {
    // Write-1-to-clear the TC (Transmission Completed) flag only
    fdcan->IR = FDCAN_IR_TC;
}