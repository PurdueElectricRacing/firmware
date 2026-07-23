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
    RCC->CCIPR &= ~RCC_CCIPR_FDCANSEL_Msk;
    RCC->CCIPR |= FDCAN_PRIV_RCC_FDCANCLKSOURCE_PCLK1;
    RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
}
 
void PHAL_FDCAN_priv_enterConfig(FDCAN_GlobalTypeDef *fdcan) {
    fdcan->CCCR &= ~FDCAN_CCCR_CSR;
    while (fdcan->CCCR & FDCAN_CCCR_CSA) {
        __asm__("nop");
    }
    fdcan->CCCR |= FDCAN_CCCR_INIT;
    while (!(fdcan->CCCR & FDCAN_CCCR_INIT)) {
        __asm__("nop");
    }
    fdcan->CCCR |= FDCAN_CCCR_CCE;
}
 
void PHAL_FDCAN_priv_exitConfig(FDCAN_GlobalTypeDef *fdcan) {
    fdcan->CCCR &= ~FDCAN_CCCR_INIT;
    while (fdcan->CCCR & FDCAN_CCCR_INIT) {
        __asm__("nop");
    }
}
 
// Builds an NBTP register for a 16-time-quantum nominal bit time (~87.5%
// sample point) at the given BRP. (BRP=2 @ 500k, BRP=1 @ 1M).
static uint32_t buildNBTP16TQ(uint32_t brp) {
    const uint32_t seg1 = 13, seg2 = 2, sjw = 2;
    return ((brp - 1U) << FDCAN_NBTP_NBRP_Pos) | ((seg1 - 1U) << FDCAN_NBTP_NTSEG1_Pos)
        | ((seg2 - 1U) << FDCAN_NBTP_NTSEG2_Pos) | ((sjw - 1U) << FDCAN_NBTP_NSJW_Pos);
}
 
uint32_t PHAL_FDCAN_priv_getNBTP(PHAL_FDCAN_BaudRate_t bit_rate) {
    switch (bit_rate) {
        case FDCAN_BAUD_500K:
            return buildNBTP16TQ(FDCAN_PRIV_KER_CLK_HZ / (500000U * 16U));
        case FDCAN_BAUD_1M:
            return buildNBTP16TQ(FDCAN_PRIV_KER_CLK_HZ / (1000000U * 16U));
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
 
static uint8_t FDCAN_dlcCodeToLen(uint8_t dlc_code) {
    static const uint8_t LEN_TABLE[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
    return LEN_TABLE[dlc_code & 0xF];
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
    uint8_t len = FDCAN_dlcCodeToLen((w1 >> 16) & 0xF);
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