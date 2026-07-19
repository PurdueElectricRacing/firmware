/**
 * @file fdcan_priv.c
 * @brief G4 FDCAN low-level implementation.
 *
 * Register/message-RAM detail
 */

#include "common/phal_G4/fdcan/fdcan.h"
#include "common/phal_G4/fdcan/fdcan_priv.h"

uint32_t PHAL_FDCAN_priv_ramBase(FDCAN_GlobalTypeDef *fdcan) {
    uint32_t offset = 0;
    if (fdcan == FDCAN2)
        offset = SRAMCAN_SIZE;
    else if (fdcan == FDCAN3)
        offset = SRAMCAN_SIZE * 2U;
    return (uint32_t)SRAMCAN_BASE + offset;
}

void PHAL_FDCAN_priv_enableClock(void) {
    RCC->CCIPR &= ~RCC_CCIPR_FDCANSEL_Msk;
    RCC->CCIPR |= RCC_FDCANCLKSOURCE_PCLK1;
    RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
}

void PHAL_FDCAN_priv_enterConfig(FDCAN_GlobalTypeDef *fdcan) {
    fdcan->CCCR &= ~FDCAN_CCCR_CSR;
    while (fdcan->CCCR & FDCAN_CCCR_CSA) {}
    fdcan->CCCR |= FDCAN_CCCR_INIT;
    while (!(fdcan->CCCR & FDCAN_CCCR_INIT)) {}
    fdcan->CCCR |= FDCAN_CCCR_CCE;
}

void PHAL_FDCAN_priv_exitConfig(FDCAN_GlobalTypeDef *fdcan) {
    fdcan->CCCR &= ~FDCAN_CCCR_INIT;
    while (fdcan->CCCR & FDCAN_CCCR_INIT) {}
}

// clang-format off
// Nominal bit timing shapes for each supported bit rate.
// 125k/250k/500k/1M all share a 16 time-quantum shape (~87.5% sample point);
// only BRP changes between them. 2M uses 8 TQ (~62.5% sample point) since
// 16 TQ would need a sub-1 BRP at this kernel clock.
static const struct {
    uint32_t baud_bps;
    uint8_t tq, seg1, seg2, sjw;
} BIT_TIMING_TABLE[] = {
    {  125000U, 16, 13, 2, 2 },
    {  250000U, 16, 13, 2, 2 },
    {  500000U, 16, 13, 2, 2 },
    { 1000000U, 16, 13, 2, 2 },
    { 2000000U,  8,  4, 3, 2 },
};
// clang-format on

bool PHAL_FDCAN_priv_getNBTP(uint32_t bit_rate, uint32_t *nbtp) {
    for (uint32_t i = 0; i < sizeof(BIT_TIMING_TABLE) / sizeof(BIT_TIMING_TABLE[0]); i++) {
        if (BIT_TIMING_TABLE[i].baud_bps != bit_rate)
            continue;

        uint32_t tq  = BIT_TIMING_TABLE[i].tq;
        uint32_t brp = FDCAN_KER_CLK_HZ / (bit_rate * tq);
        if (brp < 1 || brp > 512 || (brp * bit_rate * tq) != FDCAN_KER_CLK_HZ)
            return false; // kernel clock isn't an exact multiple for this shape

        *nbtp = ((brp - 1U) << FDCAN_NBTP_NBRP_Pos)
            | ((BIT_TIMING_TABLE[i].seg1 - 1U) << FDCAN_NBTP_NTSEG1_Pos)
            | ((BIT_TIMING_TABLE[i].seg2 - 1U) << FDCAN_NBTP_NTSEG2_Pos)
            | ((BIT_TIMING_TABLE[i].sjw - 1U) << FDCAN_NBTP_NSJW_Pos);
        return true;
    }
    return false; // unsupported bit rate
}

void PHAL_FDCAN_priv_writeStandardFilters(FDCAN_GlobalTypeDef *fdcan, uint32_t *sid_list, uint32_t num_sid) {
    volatile uint32_t *ram = (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan) + SRAMCAN_FLSSA);

    for (uint32_t i = 0; i < num_sid; i++) {
        uint32_t sid = sid_list[i] & 0x7FFU;
        // SFT=10 (classic mask), SFEC=001 (store to FIFO0), SFID1=sid, SFID2=mask (all 1s -> exact match)
        ram[i] = (2U << 30) | (1U << 27) | (sid << 16) | 0x7FFU;
    }

    fdcan->RXGFC &= ~FDCAN_RXGFC_LSS_Msk;
    fdcan->RXGFC |= (num_sid << FDCAN_RXGFC_LSS_Pos);
}

void PHAL_FDCAN_priv_writeExtendedFilters(FDCAN_GlobalTypeDef *fdcan, uint32_t *xid_list, uint32_t num_xid) {
    volatile uint32_t *ram = (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan) + SRAMCAN_FLESA);

    for (uint32_t i = 0; i < num_xid; i++) {
        uint32_t xid = xid_list[i] & 0x1FFFFFFFU;
        ram[i * 2 + 0] = (1U << 29) | xid;         // EFEC=001 (store to FIFO0), EFID1=id
        ram[i * 2 + 1] = (2U << 30) | 0x1FFFFFFFU; // EFT=10 (classic mask), mask=all 1s -> exact match
    }

    fdcan->RXGFC &= ~FDCAN_RXGFC_LSE_Msk;
    fdcan->RXGFC |= (num_xid << FDCAN_RXGFC_LSE_Pos);
    fdcan->XIDAM = 0x1FFFFFFFU;
}

void PHAL_FDCAN_priv_writeTxElement(FDCAN_GlobalTypeDef *fdcan, CanMsgTypeDef_t *msg) {
    uint32_t put = (fdcan->TXFQS & FDCAN_TXFQS_TFQPI_Msk) >> FDCAN_TXFQS_TFQPI_Pos;
    volatile uint32_t *tx =
        (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan) + SRAMCAN_TFQSA + (put * SRAMCAN_TFQ_SIZE));

    uint32_t dlc = msg->DLC > 8U ? 8U : msg->DLC;

    tx[0] = msg->IDE == 0 ? ((uint32_t)(msg->StdId & 0x7FFU) << 18) // standard ID, IDE=0
                          : ((msg->ExtId & 0x1FFFFFFFU) | (1U << 30)); // extended ID, IDE=1
    tx[1] = dlc << 16; // classic CAN: BRS=0, FDF=0, EFC=0, MM=0, RTR=0

    uint32_t d0 = 0, d1 = 0;
    for (uint32_t i = 0; i < dlc && i < 4U; i++)
        d0 |= (uint32_t)msg->Data[i] << (8U * i);
    for (uint32_t i = 4; i < dlc; i++)
        d1 |= (uint32_t)msg->Data[i] << (8U * (i - 4U));
    tx[2] = d0;
    tx[3] = d1;

    fdcan->TXBAR = (1U << put);
}

static inline uint8_t dlcCodeToLen(uint8_t dlc_code) {
    static const uint8_t LEN_TABLE[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
    return LEN_TABLE[dlc_code & 0xF];
}

bool PHAL_FDCAN_priv_readRxElement(FDCAN_GlobalTypeDef *fdcan, CanMsgTypeDef_t *msg) {
    uint32_t f0s = fdcan->RXF0S;
    if ((f0s & FDCAN_RXF0S_F0FL_Msk) == 0)
        return false; // FIFO0 empty

    // If FIFO0 has overflowed with overwrite mode on, drop the oldest first.
    if ((f0s & FDCAN_RXF0S_F0F_Msk) && (fdcan->RXGFC & FDCAN_RXGFC_F0OM)) {
        fdcan->RXF0A = (f0s & FDCAN_RXF0S_F0GI_Msk) >> FDCAN_RXF0S_F0GI_Pos;
        f0s          = fdcan->RXF0S;
        if ((f0s & FDCAN_RXF0S_F0FL_Msk) == 0)
            return false;
    }

    uint32_t get = (f0s & FDCAN_RXF0S_F0GI_Msk) >> FDCAN_RXF0S_F0GI_Pos;
    volatile uint32_t *rx =
        (volatile uint32_t *)(PHAL_FDCAN_priv_ramBase(fdcan) + SRAMCAN_RF0SA + (get * SRAMCAN_RF0_SIZE));

    uint32_t w0 = rx[0];
    uint32_t w1 = rx[1];

    *msg     = (CanMsgTypeDef_t) {0};
    msg->Bus = fdcan;
    if (w0 & (1U << 30)) {
        msg->IDE   = 1;
        msg->ExtId = w0 & 0x1FFFFFFFU;
    } else {
        msg->IDE   = 0;
        msg->StdId = (uint16_t)((w0 >> 18) & 0x7FFU);
        msg->ExtId = msg->StdId;
    }

    uint8_t len = dlcCodeToLen((w1 >> 16) & 0xF);
    if (len > sizeof(msg->Data))
        len = sizeof(msg->Data);
    msg->DLC = len;

    const uint8_t *payload = (const uint8_t *)&rx[2];
    for (uint8_t i = 0; i < len; i++)
        msg->Data[i] = payload[i];

    fdcan->RXF0A = get; // pop the element we just read
    return true;
}