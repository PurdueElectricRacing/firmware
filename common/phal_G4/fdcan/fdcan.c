/**
 * @file fdcan.c
 * @author Eileen Yoon
 * @brief G4 FDCAN
 * @version 0.1
 * @date 2025-08-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "common/phal_G4/fdcan/fdcan.h"

#include "common/phal_G4/fdcan/fdcan_priv.h"

static uint32_t PHAL_FDCAN_get_ram_base(FDCAN_GlobalTypeDef* fdcan) {
    uint32_t base = (uint32_t)SRAMCAN_BASE;
    if (fdcan == FDCAN2) {
        base += (uint32_t)SRAMCAN_SIZE;
    } else if (fdcan == FDCAN3) {
        base += (uint32_t)(SRAMCAN_SIZE * 2U);
    }
    return base;
}

// Returns 0 on success
//       -1 unsupported baud
//       -2 bad args or BRP out of range (1..512)
//       -3 ker_hz not an EXACT multiple of (baud * TQ)
static int PHAL_FDCAN_makeNBTP(uint32_t ker_hz, uint32_t baud_bps, uint32_t* nbtp_out) {
    if (!nbtp_out || ker_hz == 0u)
        return -2;

    uint8_t tq, seg1, seg2, sjw;

    switch (baud_bps) {
        case 125000u: // 16 TQ, SP=87.5%
        case 250000u:
        case 500000u:
        case 1000000u:
            tq   = 16;
            seg1 = 13;
            seg2 = 2;
            sjw  = 2;
            break;

        case 2000000u: // 8 TQ, SP≈62.5% (1 + 4)/8
            tq   = 8;
            seg1 = 4;
            seg2 = 3;
            sjw  = 2;
            break;

        default:
            return -1; // unsupported
    }

    if (sjw > seg2)
        sjw = seg2; // hardware requires SJW ≤ TSEG2

    // BRP = ker_hz / (baud * tq)  (must be integer)
    uint64_t denom = (uint64_t)baud_bps * (uint64_t)tq;
    if (denom == 0u)
        return -2;

    uint32_t brp = (uint32_t)(ker_hz / denom);
    if (brp < 1u || brp > 512u)
        return -2;

    // Enforce exact match so the nominal bit-rate is precise
    if ((uint64_t)brp * denom != (uint64_t)ker_hz) {
        return -3; // pick a different ker_hz (or use CKDIV) or another TQ shape
    }

    *nbtp_out =
        ((uint32_t)(brp - 1u) << FDCAN_NBTP_NBRP_Pos) | ((uint32_t)(seg1 - 1u) << FDCAN_NBTP_NTSEG1_Pos) | ((uint32_t)(seg2 - 1u) << FDCAN_NBTP_NTSEG2_Pos) | ((uint32_t)(sjw - 1u) << FDCAN_NBTP_NSJW_Pos);

    return 0;
}

static void PHAL_FDCAN_setExtendedFilter(FDCAN_GlobalTypeDef* fdcan, uint32_t num_xid, uint32_t* xid_list) {
    if (!num_xid || num_xid > MAX_NUM_XID_FILTER) {
        return;
    }

    volatile uint32_t* ram = (volatile uint32_t*)(PHAL_FDCAN_get_ram_base(fdcan) + SRAMCAN_FLESA);
    // Program num_xid filter elements, 2 words each
    for (uint32_t i = 0; i < num_xid; i++) {
        uint32_t xid = xid_list[i] & 0x1FFFFFFFu;
        // Word 0: EFEC=001 (store to FIFO0), EFID1 = id
        ram[i * 2 + 0] = (1u << 29) | xid;
        // Word 1: EFT=10 (classic mask), mask=all ones -> exact match
        ram[i * 2 + 1] = (2u << 30) | 0x1FFFFFFFu;
    }

    // Set number of extended filters
    fdcan->RXGFC &= ~FDCAN_RXGFC_LSE_Msk;
    fdcan->RXGFC |= ((num_xid & 0x7u) << FDCAN_RXGFC_LSE_Pos);

    fdcan->XIDAM = 0x1FFFFFFFu;
}

static void PHAL_FDCAN_setStandardFilter(FDCAN_GlobalTypeDef* fdcan,
                                         uint32_t num_sid,
                                         uint32_t* sid_list) {
    if (!num_sid || num_sid > MAX_NUM_SID_FILTER) {
        return;
    }

    volatile uint32_t* ram = (volatile uint32_t*)(PHAL_FDCAN_get_ram_base(fdcan) + SRAMCAN_FLSSA);

    // Program standard filters (1 word each)
    for (uint32_t i = 0; i < num_sid; i++) {
        uint32_t sid = sid_list[i] & 0x7FFu; // 11-bit
        // exact match (mask = 0x7FF), store to FIFO0
        // SFT=10 (classic mask), SFEC=001 (FIFO0), SFID1 = sid, SFID2 = 0x7FF
        ram[i] = (2u << 30) | (1u << 27) | (sid << 16) | 0x7FFu;
    }

    // Set number of standard filters
    fdcan->RXGFC &= ~(FDCAN_RXGFC_LSS_Msk);
    fdcan->RXGFC |= ((num_sid & 0x7Fu) << FDCAN_RXGFC_LSS_Pos);
}

void PHAL_FDCAN_setFilters(FDCAN_GlobalTypeDef* fdcan, uint32_t* sid_list, uint32_t num_sid, uint32_t* xid_list, uint32_t num_xid) {
    fdcan->CCCR |= FDCAN_CCCR_INIT;
    while ((fdcan->CCCR & FDCAN_CCCR_INIT) == 0) {}
    fdcan->CCCR |= FDCAN_CCCR_CCE;

    fdcan->RXGFC = (FDCAN_REJECT << FDCAN_RXGFC_ANFS_Pos)
        | (FDCAN_REJECT << FDCAN_RXGFC_ANFE_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFS_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFE_Pos);

    PHAL_FDCAN_setStandardFilter(fdcan, num_sid, sid_list);
    PHAL_FDCAN_setExtendedFilter(fdcan, num_xid, xid_list);

    fdcan->CCCR &= ~FDCAN_CCCR_INIT;
    while ((fdcan->CCCR & FDCAN_CCCR_INIT) != 0) {}
}

bool PHAL_FDCAN_init(FDCAN_GlobalTypeDef* fdcan, bool test_mode, uint32_t bit_rate) {
    // Enable clocks
    RCC->CCIPR &= ~RCC_CCIPR_FDCANSEL_Msk;
    RCC->CCIPR |= RCC_FDCANCLKSOURCE_PCLK1;
    RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN; // Always use PCLK1

    // Enable port B clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // Ensure peripheral is out of sleep
    fdcan->CCCR &= ~FDCAN_CCCR_CSR;
    while (fdcan->CCCR & FDCAN_CCCR_CSA) {}
    // Enter init
    fdcan->CCCR |= FDCAN_CCCR_INIT;
    while ((fdcan->CCCR & FDCAN_CCCR_INIT) == 0) {}
    // Allow configuration
    fdcan->CCCR |= FDCAN_CCCR_CCE;

    /* Classic CAN (no FD) */
    fdcan->CCCR &= ~(FDCAN_CCCR_FDOE | FDCAN_CCCR_BRSE | FDCAN_CCCR_TXP);
    fdcan->CCCR |= (FDCAN_CCCR_DAR /* Auto retransmission: disable */
                    | FDCAN_CCCR_PXHD); /* Protocol exception: disable */
    fdcan->CCCR &= ~(FDCAN_CCCR_MON | FDCAN_CCCR_ASM | FDCAN_CCCR_TEST);

    fdcan->CCCR &= ~(FDCAN_CCCR_DAR); // Auto retransmission
    fdcan->CCCR |= FDCAN_CCCR_TXP; // Transmit pause

    uint32_t nbtp;
    // TODO: Get clock frequency
    if (PHAL_FDCAN_makeNBTP(/*ker_hz*/ 16000000u, bit_rate, &nbtp) == 0) {
        fdcan->NBTP = nbtp; // do this while INIT=1 and CCE=1
    } else {
        return false;
    }
    // No FD/DBTP yet

    // FIFOs
    fdcan->TXBC &= ~FDCAN_TXBC_TFQM; // Tx FIFO mode

    // Clear interrupt flags
    fdcan->IR |= 0xFFFFFFFF;
    // Enable RX FIFO0 interrupts
    fdcan->IE |= FDCAN_IE_RF0NE // New message
        | FDCAN_IE_RF0FE // FIFO full (optional)
        | FDCAN_IE_RF0LE; // Message lost (optional)
    fdcan->ILS = 0; // Route to interrupt line 0
    fdcan->ILE = FDCAN_ILE_EINT0; // Enable line 0

    // Mode
    fdcan->CCCR &= ~(FDCAN_CCCR_MON | FDCAN_CCCR_TEST | FDCAN_CCCR_ASM);
    fdcan->TEST &= ~FDCAN_TEST_LBCK;
    if (test_mode) {
        fdcan->CCCR |= FDCAN_CCCR_TEST | FDCAN_CCCR_MON;
        fdcan->TEST |= FDCAN_TEST_LBCK;
    }

    // Filters (accept all by default)
    fdcan->RXGFC = (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_ANFS_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_ANFE_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFS_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFE_Pos);

    fdcan->CCCR &= ~FDCAN_CCCR_INIT;
    while (fdcan->CCCR & FDCAN_CCCR_INIT) {}

    return true;
}

void PHAL_FDCAN_send(CanMsgTypeDef_t* msg) {
    FDCAN_GlobalTypeDef* fdcan = msg->Bus;
    // TX FIFO/Queue full?
    if (fdcan->TXFQS & FDCAN_TXFQS_TFQF) {
        // TODO: count drops
        return;
    }

    uint32_t ram_base = PHAL_FDCAN_get_ram_base(fdcan);
    uint32_t put      = (fdcan->TXFQS & FDCAN_TXFQS_TFQPI_Msk) >> FDCAN_TXFQS_TFQPI_Pos;

    volatile uint32_t* tx =
        (uint32_t*)((ram_base + SRAMCAN_TFQSA) + (put * SRAMCAN_TFQ_SIZE));
    uint32_t w0;
    if (msg->IDE == 0) {
        // Standard ID in [28:18], IDE=0
        w0 = ((uint32_t)(msg->StdId & 0x7FFu) << 18);
    } else {
        // Extended ID in [28:0], set IDE/XTD bit (bit 30)
        w0 = (msg->ExtId & 0x1FFFFFFFu) | (1u << 30);
    }
    tx[0] = w0;

    // 4) Header word 1: DLC at [19:16], classic CAN: BRS=0, FDF=0, EFC=0, MM=0, RTR=0
    uint32_t dlc = (msg->DLC & 0xFu);
    if (dlc > 8u)
        dlc = 8u; // assumes TXESC TBDS = 8 bytes
    tx[1] = (dlc << 16);

    uint32_t d0 = 0;
    uint32_t d1 = 0;
    if (dlc > 0)
        d0 |= (uint32_t)msg->Data[0];
    if (dlc > 1)
        d0 |= (uint32_t)msg->Data[1] << 8;
    if (dlc > 2)
        d0 |= (uint32_t)msg->Data[2] << 16;
    if (dlc > 3)
        d0 |= (uint32_t)msg->Data[3] << 24;
    if (dlc > 4)
        d1 |= (uint32_t)msg->Data[4];
    if (dlc > 5)
        d1 |= (uint32_t)msg->Data[5] << 8;
    if (dlc > 6)
        d1 |= (uint32_t)msg->Data[6] << 16;
    if (dlc > 7)
        d1 |= (uint32_t)msg->Data[7] << 24;

    tx[2] = d0;
    tx[3] = d1;
    tx[4] = 0; // pad (8-byte elements)

    fdcan->TXBAR = (1u << put);
}

bool PHAL_FDCAN_txFifoFree(FDCAN_GlobalTypeDef* fdcan) {
    return !(fdcan->TXFQS & FDCAN_TXFQS_TFQF);
}

static inline uint8_t dlc_to_len(uint8_t dlc) {
    static const uint8_t map[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
    return map[dlc & 0xF];
}

// Return 0 on success; -2 if FIFO empty; -1 on bad args
static int PHAL_FDCAN_getRxMessage(FDCAN_GlobalTypeDef* fdcan, CanMsgTypeDef_t* m) {
    if (!fdcan || !m)
        return -1;

    // Empty?
    uint32_t f0s = fdcan->RXF0S;
    if ((f0s & FDCAN_RXF0S_F0FL_Msk) == 0u)
        return -2;

    // If FIFO0 is full AND overwrite mode is enabled, drop the oldest first
    if ((f0s & FDCAN_RXF0S_F0F_Msk) && (fdcan->RXGFC & FDCAN_RXGFC_F0OM)) {
        uint32_t gi_drop = (f0s & FDCAN_RXF0S_F0GI_Msk) >> FDCAN_RXF0S_F0GI_Pos;
        fdcan->RXF0A     = gi_drop; // discard oldest
        // Re-check after dropping
        f0s = fdcan->RXF0S;
        if ((f0s & FDCAN_RXF0S_F0FL_Msk) == 0u)
            return -2;
    }

    // Element index to read now
    uint32_t get = (f0s & FDCAN_RXF0S_F0GI_Msk) >> FDCAN_RXF0S_F0GI_Pos;

    // Message RAM address (your fixed layout macros)
    uintptr_t ram_base = PHAL_FDCAN_get_ram_base(fdcan);
    volatile uint32_t* rx =
        (uint32_t*)((ram_base + SRAMCAN_RF0SA) + (get * SRAMCAN_RF0_SIZE));

    uint32_t w0 = rx[0]; // ID + XTD/RTR/ESI
    uint32_t w1 = rx[1]; // TS + DLC/BRS/FDF/FIDX/ANMF

    // Fill your CanMsgTypeDef_t
    *m     = (CanMsgTypeDef_t) {0};
    m->Bus = fdcan;

    if (w0 & (1u << 30)) { // XTD (IDE) = 1 → extended
        m->IDE   = 1;
        m->ExtId = (w0 & 0x1FFFFFFFu);
    } else { // standard
        m->IDE   = 0;
        m->StdId = (uint16_t)((w0 >> 18) & 0x7FFu);
        m->ExtId = m->StdId; // your parser expects ExtId too
    }

    // DLC code → bytes; cap to our 8-byte buffer
    uint8_t dlc_code  = (uint8_t)((w1 >> 16) & 0xF);
    uint8_t len_bytes = dlc_to_len(dlc_code);
    if (len_bytes > sizeof m->Data)
        len_bytes = sizeof m->Data;
    m->DLC = len_bytes;

    // Copy payload (rx[2].. as bytes)
    const uint8_t* p = (const uint8_t*)&rx[2];
    for (uint8_t i = 0; i < len_bytes; ++i)
        m->Data[i] = p[i];

    // Pop this FIFO0 element
    fdcan->RXF0A = get;

    return 0;
}

void PHAL_FDCAN_RX_IRQHandler(FDCAN_GlobalTypeDef* fdcan) {
    uint32_t ir = fdcan->IR;
    if (ir & FDCAN_IR_RF0L) {
        fdcan->IR = FDCAN_IR_RF0L;
        // TODO message lost
    }

    if (ir & FDCAN_IR_RF0F) {
        fdcan->IR = FDCAN_IR_RF0F;
        // TODO FIFO full
    }

    if (ir & FDCAN_IR_RF0N) {
        CanMsgTypeDef_t m;
        while (PHAL_FDCAN_getRxMessage(fdcan, &m) == 0) {
            PHAL_FDCAN_rxCallback(&m); // or enqueue
        }
        fdcan->IR = FDCAN_IR_RF0N; // clear after draining
    }
}

void __attribute__((weak)) PHAL_FDCAN_rxCallback(CanMsgTypeDef_t* msg) {
    (void)msg;
}

void FDCAN1_IT0_IRQHandler(void) {
    PHAL_FDCAN_RX_IRQHandler(FDCAN1);
}

void FDCAN2_IT0_IRQHandler(void) {
    PHAL_FDCAN_RX_IRQHandler(FDCAN2);
}

void FDCAN3_IT0_IRQHandler(void) {
    PHAL_FDCAN_RX_IRQHandler(FDCAN3);
}
