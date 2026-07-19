#ifndef __PHAL_G4_FDCAN_H__
#define __PHAL_G4_FDCAN_H__

#include <stdbool.h>
#include <stdint.h>

#include "common/phal_G4/phal_G4.h"

#define MAX_NUM_SID_FILTER (28)
#define MAX_NUM_XID_FILTER (8)

/**
 * @brief A classic CAN frame, used for both TX and RX.
 *
 * On RX (delivered via PHAL_FDCAN_rxCallback), `Bus` identifies which
 * peripheral the frame arrived on. On TX (passed to PHAL_FDCAN_send),
 * `Bus` selects which peripheral transmits it.
 */
typedef struct {
    FDCAN_GlobalTypeDef *Bus; /*!< Peripheral instance (FDCAN1/2/3). */
    uint16_t StdId;           /*!< 11-bit standard identifier (used if IDE == 0). */
    uint32_t ExtId;           /*!< 29-bit extended identifier (used if IDE == 1). */
    uint32_t IDE;             /*!< 0 = standard frame, 1 = extended frame. */
    uint32_t DLC;             /*!< Payload length in bytes, 0-8. */
    uint8_t Data[8];          /*!< Payload bytes. */
} CanMsgTypeDef_t;

/**
 * @brief Initialize an FDCAN peripheral for classic (non-FD) CAN operation.
 *
 * This configures, and hides the details of:
 *  - FDCAN kernel clock selection (PCLK1) and peripheral clock enable
 *  - Nominal bit timing for one of a small set of supported bit rates,
 *    chosen for an ~87.5% sample point (62.5% at 2 Mbit/s)
 *  - Classic CAN mode (FD/BRS off), auto-retransmission enabled, TX pause on
 *  - TX FIFO (not queue) mode
 *  - RX FIFO0-new-message and TX-complete interrupts, routed to
 *    interrupt lines 0 and 1 respectively
 *  - A default "accept everything into RX FIFO0" filter policy, which
 *    PHAL_FDCAN_setFilters can narrow later
 *
 * @param fdcan peripheral instance (FDCAN1/2/3)
 * @param loopback true to enable internal loopback + bus monitoring, so the
 *                 peripheral can be exercised with no other node on the bus
 * @param bit_rate desired bit rate in bps; must be one of
 *                 125000, 250000, 500000, 1000000, 2000000
 * @return true on success; false if bit_rate is not one of the supported
 *         values above
 */
bool PHAL_FDCAN_init(FDCAN_GlobalTypeDef *fdcan, bool loopback, uint32_t bit_rate);

/**
 * @brief Configure exact-match acceptance filters for RX FIFO0.
 *
 * Every ID in sid_list/xid_list is matched exactly and accepted into RX
 * FIFO0. Any frame whose ID is not in either list is rejected. Calling
 * this replaces any filter configuration set by a previous call (or by
 * the "accept everything" default from PHAL_FDCAN_init).
 *
 * @param fdcan peripheral instance
 * @param sid_list array of standard (11-bit) IDs to accept; may be NULL if num_sid == 0
 * @param num_sid number of entries in sid_list, up to MAX_NUM_SID_FILTER
 * @param xid_list array of extended (29-bit) IDs to accept; may be NULL if num_xid == 0
 * @param num_xid number of entries in xid_list, up to MAX_NUM_XID_FILTER
 * @return true on success; false if num_sid or num_xid exceeds its max
 */
bool PHAL_FDCAN_setFilters(
    FDCAN_GlobalTypeDef *fdcan,
    uint32_t *sid_list,
    uint32_t num_sid,
    uint32_t *xid_list,
    uint32_t num_xid
);

/**
 * @brief Queue a frame for transmission.
 *
 * Non-blocking: if the TX FIFO is already full this returns false
 * immediately rather than waiting. Use PHAL_FDCAN_txFifoFree() to poll,
 * or retry from PHAL_FDCAN_txCallback() once a slot frees up.
 *
 * @param msg frame to send; msg->Bus selects the peripheral
 * @return true if the frame was queued; false if the TX FIFO is full
 */
bool PHAL_FDCAN_send(CanMsgTypeDef_t *msg);

/**
 * @brief Check whether the TX FIFO has at least one free slot.
 * @param fdcan peripheral instance
 * @return true if a slot is free
 */
bool PHAL_FDCAN_txFifoFree(FDCAN_GlobalTypeDef *fdcan);

/**
 * @brief Weak callback fired once per received frame.
 *
 * Called from FDCANx_IT0_IRQHandler (interrupt context) for every frame
 * popped from RX FIFO0. Override this in application code; the default
 * implementation does nothing.
 *
 * @param msg the received frame (valid only for the duration of the call)
 */
extern void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg);

/**
 * @brief Weak callback fired when a queued frame finishes transmitting.
 *
 * Called from FDCANx_IT1_IRQHandler (interrupt context). Override this in
 * application code; the default implementation does nothing.
 *
 * @param fdcan the peripheral instance that completed a transmission
 */
extern void PHAL_FDCAN_txCallback(FDCAN_GlobalTypeDef *fdcan);

#define AF_NUM_FDCAN1 (9)
#define AF_NUM_FDCAN2 (9)
#define AF_NUM_FDCAN3 (11)

// FDCAN1 GPIO definitions (PA11/PA12 or PB8/PB9)
#define GPIO_INIT_FDCAN1RX_PA11 \
    GPIO_INIT_AF(GPIOA, \
                 11, \
                 AF_NUM_FDCAN1, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_OPEN_DRAIN, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN1TX_PA12 \
    GPIO_INIT_AF(GPIOA, \
                 12, \
                 AF_NUM_FDCAN1, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_PUSH_PULL, \
                 GPIO_INPUT_OPEN_DRAIN)

// !!! double check the AF number for PB8/PB9, not validated yet
#define GPIO_INIT_FDCAN1RX_PB8 \
    GPIO_INIT_AF(GPIOB, \
                 8, \
                 AF_NUM_FDCAN1, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_OPEN_DRAIN, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN1TX_PB9 \
    GPIO_INIT_AF(GPIOB, \
                 9, \
                 AF_NUM_FDCAN1, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_PUSH_PULL, \
                 GPIO_INPUT_OPEN_DRAIN)

// FDCAN2 GPIO definitions (PB12/PB13 or PB5/PB6)
#define GPIO_INIT_FDCAN2RX_PB12 \
    GPIO_INIT_AF(GPIOB, \
                 12, \
                 AF_NUM_FDCAN2, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_OPEN_DRAIN, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN2TX_PB13 \
    GPIO_INIT_AF(GPIOB, \
                 13, \
                 AF_NUM_FDCAN2, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_PUSH_PULL, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN2RX_PB5 \
    GPIO_INIT_AF(GPIOB, \
                 5, \
                 AF_NUM_FDCAN2, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_OPEN_DRAIN, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN2TX_PB6 \
    GPIO_INIT_AF(GPIOB, \
                 6, \
                 AF_NUM_FDCAN2, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_PUSH_PULL, \
                 GPIO_INPUT_OPEN_DRAIN)

// FDCAN3 GPIO definitions (PA8/PB4 or PA15/PB3)
#define GPIO_INIT_FDCAN3RX_PA8 \
    GPIO_INIT_AF(GPIOA, \
                 8, \
                 AF_NUM_FDCAN3, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_OPEN_DRAIN, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN3TX_PB4 \
    GPIO_INIT_AF(GPIOB, \
                 4, \
                 AF_NUM_FDCAN3, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_PUSH_PULL, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN3TX_PA15 \
    GPIO_INIT_AF(GPIOA, \
                 15, \
                 AF_NUM_FDCAN3, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_OPEN_DRAIN, \
                 GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN3TX_PB3 \
    GPIO_INIT_AF(GPIOB, \
                 3, \
                 AF_NUM_FDCAN3, \
                 GPIO_OUTPUT_ULTRA_SPEED, \
                 GPIO_OUTPUT_PUSH_PULL, \
                 GPIO_INPUT_OPEN_DRAIN)

#endif // __PHAL_G4_FDCAN_H__