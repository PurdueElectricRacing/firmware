#include "common/can_library/generated/can_router.h"
#include "common/can_library/can_common.h"
#include "common/queue/queue.h"
#include "common/psched/psched.h"

// common data structures
can_data_t can_data;
can_stats_t can_stats;

#if defined(STM32F407xx) || defined(STM32F732xx)


// todo mailbox based implementation here
q_handle_t q_tx_can[NUM_CAN_PERIPHERALS][CAN_TX_MAILBOX_CNT];
q_handle_t q_rx_can;

uint32_t can_mbx_last_send_time[NUM_CAN_PERIPHERALS][CAN_TX_MAILBOX_CNT];


void CAN_enqueue_tx(CanMsgTypeDef_t *msg) {
    uint32_t t = 0;
    /* Don't use multiple mailboxes to guarantee in-order transmission */
    while (!PHAL_txMailboxFree(CAN1, 0) && (t++ < CAN_TX_BLOCK_TIMEOUT))
        ;
    if (t < CAN_TX_BLOCK_TIMEOUT)
        PHAL_txCANMessage(msg, 0);
    else
        can_stats[BUS_ID_CAN1].tx_of++;

}

void CAN_tx_update() {
CanMsgTypeDef_t tx_msg;
    for (uint8_t i = 0; i < CAN_TX_MAILBOX_CNT; ++i)
    {
#ifdef USE_CAN1
        // Handle CAN1
        if(PHAL_txMailboxFree(CAN1, i))
        {
            if (qReceive(&q_tx_can[CAN1_IDX][i], &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
            {
                PHAL_txCANMessage(&tx_msg, i);
                can_mbx_last_send_time[CAN1_IDX][i] = sched.os_ticks;
            }
        }
        else if (sched.os_ticks - can_mbx_last_send_time[CAN1_IDX][i] > CAN_TX_TIMEOUT_MS)
        {
            PHAL_txCANAbort(CAN1, i); // aborts tx and empties the mailbox
            can_stats.can_peripheral_stats[CAN1_IDX].tx_fail++;
        }
#endif
# ifdef USE_CAN2
        // Handle CAN2
        if(PHAL_txMailboxFree(CAN2, i))
        {
            if (qReceive(&q_tx_can[CAN2_IDX][i], &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
            {
                PHAL_txCANMessage(&tx_msg, i);
                can_mbx_last_send_time[CAN2_IDX][i] = sched.os_ticks;
            }
        }
        else if (sched.os_ticks - can_mbx_last_send_time[CAN2_IDX][i] > CAN_TX_TIMEOUT_MS)
        {
            PHAL_txCANAbort(CAN2, i); // aborts tx and empties the mailbox
            can_stats.can_peripheral_stats[CAN2_IDX].tx_fail++;
        }
#endif
    }

}

bool CAN_prepare_filter_config(CAN_TypeDef *can) {
    can->MCR |= CAN_MCR_INRQ; // Enter back into INIT state (required for changing scale)
    uint32_t timeout = 0;
    while (!(can->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
        return false;

    can->FMR |= CAN_FMR_FINIT; // Enter init mode for filter banks
    can->FM1R |= 0x07FFFFFF; // Set banks 0-27 to id mode
    can->FS1R |= 0x07FFFFFF; // Set banks 0-27 to 32-bit scale

    return true;
}

bool CAN_exit_filter_config(CAN_TypeDef *can) {
    can->FMR &= ~CAN_FMR_FINIT; // Enable Filters (exit filter init mode)

    // Enter back into NORMAL mode
    can->MCR &= ~CAN_MCR_INRQ;
    uint32_t timeout = 0;
    while ((can->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;

    return timeout != PHAL_CAN_INIT_TIMEOUT;
}


bool CAN_library_init() {
    for (uint8_t can_periph = 0; can_periph < NUM_CAN_PERIPHERALS; can_periph++)
    {
      for (uint8_t mbx = 0; mbx < CAN_TX_MAILBOX_CNT; mbx++)
      {
        qConstruct(&q_tx_can[can_periph][mbx], sizeof(CanMsgTypeDef_t));
        can_mbx_last_send_time[can_periph][mbx] = 0;
      }
    }
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    can_stats = (can_stats_t){0};
    
    #ifdef USE_CAN1
    if (!CAN_prepare_filter_config(CAN1)) {
        return false;
    }
    
    CAN1_set_filters(); // from NODE.h

    if (!CAN_exit_filter_config(CAN1)) {
        return false;
    }
    #endif

    #ifdef USE_CAN2
    if (!CAN_prepare_filter_config(CAN2)) {
        return false;
    }
    
    CAN2_set_filters(); // from NODE.h

    if (!CAN_exit_filter_config(CAN2)) {
        return false;
    }
    #endif

    return true;
}

#elif defined(STM32G474xx)

// todo ram based implementation here



void init_CAN1() {

}

#else
#error "Unsupported architecture"
#endif