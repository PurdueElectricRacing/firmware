/**
 * @file fdcan.c
 * @brief G4 FDCAN public API implementation
 * @author Millan Kumar (kumar798@purdue.edu)
 * 
 * Logic level code. All register/message-RAM detail lives in fdcan_priv.c.
 */

#include "common/phal_G4/fdcan/fdcan.h"

#include "common/phal_G4/fdcan/fdcan_priv.h"


void PHAL_FDCAN_init(FDCAN_GlobalTypeDef *fdcan, PHAL_FDCAN_BaudRate_t bit_rate) {
    PHAL_FDCAN_priv_enableClock();
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
 
    PHAL_FDCAN_priv_enterConfig(fdcan);
 
    // Classic CAN config
    fdcan->CCCR &= ~(FDCAN_CCCR_FDOE | FDCAN_CCCR_BRSE | FDCAN_CCCR_MON | FDCAN_CCCR_ASM | FDCAN_CCCR_TEST);
    fdcan->CCCR |= FDCAN_CCCR_PXHD;  // disable protocol exception handling
    fdcan->CCCR &= ~FDCAN_CCCR_DAR;  // enable auto-retransmission
    fdcan->CCCR |= FDCAN_CCCR_TXP;   // enable transmit pause
    fdcan->TEST &= ~FDCAN_TEST_LBCK; // disable loopback
 
    // Time quantum nominal bit
    fdcan->NBTP  = PHAL_FDCAN_priv_getNBTP(bit_rate);

    // FIFO mode for TX buffer
    fdcan->TXBC &= ~FDCAN_TXBC_TFQM;
 
    // RX new message -> line 0, TX complete -> line 1
    fdcan->ILS     = FDCAN_ILS_SMSG;
    fdcan->ILE     = FDCAN_ILE_EINT0 | FDCAN_ILE_EINT1;
    fdcan->IR      = FDCAN_IR_RF0N | FDCAN_IR_TC; // clear any stale flags
    fdcan->IE     |= FDCAN_IE_RF0NE | FDCAN_IE_TCE;
    fdcan->TXBTIE  = 0xFFFFFFFFU; // TX complete interrupt for every TX buffer
 
    // Default filters: accept everything
    fdcan->RXGFC = (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_ANFS_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_ANFE_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFS_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFE_Pos);
 
    PHAL_FDCAN_priv_exitConfig(fdcan);
}
 
bool PHAL_FDCAN_setFilters(FDCAN_GlobalTypeDef *fdcan,
                           uint32_t *sid_list,
                           uint32_t num_sid,
                           uint32_t *xid_list,
                           uint32_t num_xid) {
    bool sid_exceeded = num_sid > PHAL_FDCAN_MAX_NUM_SID_FILTER;
    bool xid_exceeded = num_xid > PHAL_FDCAN_MAX_NUM_XID_FILTER;
    if (sid_exceeded || xid_exceeded) {
        return false;
    }
 
    PHAL_FDCAN_priv_enterConfig(fdcan);
 
    // Exact match: anything not in the list is rejected
    fdcan->RXGFC = (FDCAN_REJECT << FDCAN_RXGFC_ANFS_Pos) | (FDCAN_REJECT << FDCAN_RXGFC_ANFE_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFS_Pos)
        | (FDCAN_ACCEPT_IN_RX_FIFO0 << FDCAN_RXGFC_RRFE_Pos);
 
    if (num_sid > 0) {
        PHAL_FDCAN_priv_writeStandardFilters(fdcan, sid_list, num_sid);
    }
    if (num_xid) {
        PHAL_FDCAN_priv_writeExtendedFilters(fdcan, xid_list, num_xid);
    }
 
    PHAL_FDCAN_priv_exitConfig(fdcan);
    return true;
}
 
bool PHAL_FDCAN_txFifoFree(FDCAN_GlobalTypeDef *fdcan) {
    return !(fdcan->TXFQS & FDCAN_TXFQS_TFQF);
}
 
bool PHAL_FDCAN_send(CanMsgTypeDef_t *msg) {
    if (!PHAL_FDCAN_txFifoFree(msg->Bus)) {
        return false;
    }
 
    PHAL_FDCAN_priv_writeTxElement(msg->Bus, msg);
    return true;
}
 
[[gnu::always_inline]]
static inline void PHAL_FDCAN_rxIRQHandler(FDCAN_GlobalTypeDef *fdcan) {
    if (!(fdcan->IR & FDCAN_IR_RF0N)) {
        return;
    }
    fdcan->IR = FDCAN_IR_RF0N;
 
    CanMsgTypeDef_t msg;
    while (PHAL_FDCAN_priv_readRxElement(fdcan, &msg)) {
        PHAL_FDCAN_rxCallback(&msg);
    }
}
 
[[gnu::always_inline]]
static inline void PHAL_FDCAN_txIRQHandler(FDCAN_GlobalTypeDef *fdcan) {
    if (!(fdcan->IR & FDCAN_IR_TC)) {
        return;
    }
    fdcan->IR = FDCAN_IR_TC;

    PHAL_FDCAN_txCallback(fdcan);
}
 
[[gnu::weak]]
void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg) {
    (void)msg;
}
 
[[gnu::weak]]
void PHAL_FDCAN_txCallback(FDCAN_GlobalTypeDef *fdcan) {
    (void)fdcan;
}
 
void FDCAN1_IT0_IRQHandler(void) {
    PHAL_FDCAN_rxIRQHandler(FDCAN1);
}
 
void FDCAN1_IT1_IRQHandler(void) {
    PHAL_FDCAN_txIRQHandler(FDCAN1);
}
 
void FDCAN2_IT0_IRQHandler(void) {
    PHAL_FDCAN_rxIRQHandler(FDCAN2);
}
 
void FDCAN2_IT1_IRQHandler(void) {
    PHAL_FDCAN_txIRQHandler(FDCAN2);
}
 
void FDCAN3_IT0_IRQHandler(void) {
    PHAL_FDCAN_rxIRQHandler(FDCAN3);
}
 
void FDCAN3_IT1_IRQHandler(void) {
    PHAL_FDCAN_txIRQHandler(FDCAN3);
}