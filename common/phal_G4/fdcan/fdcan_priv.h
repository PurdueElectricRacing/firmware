#ifndef __PHAL_G4_FDCAN_PRIV_H__
#define __PHAL_G4_FDCAN_PRIV_H__

#include <stdbool.h>
#include <stdint.h>

#include "common/phal_G4/phal_G4.h"

// This peripheral's kernel clock. FDCAN is always clocked from PCLK1
// (see PHAL_FDCAN_priv_enableClock); update this if PCLK1 ever changes.
#define FDCAN_KER_CLK_HZ (16000000U)

#define RCC_FDCANCLKSOURCE_HSE   0x00000000U
#define RCC_FDCANCLKSOURCE_PLL   RCC_CCIPR_FDCANSEL_0
#define RCC_FDCANCLKSOURCE_PCLK1 RCC_CCIPR_FDCANSEL_1

#define FDCAN_ACCEPT_IN_RX_FIFO0 ((uint32_t)0x00000000U) /*!< Accept in Rx FIFO 0 */
#define FDCAN_ACCEPT_IN_RX_FIFO1 ((uint32_t)0x00000001U) /*!< Accept in Rx FIFO 1 */
#define FDCAN_REJECT             ((uint32_t)0x00000002U) /*!< Reject              */

// --- Message RAM layout (shared fixed layout, replicated per-instance) ---

#define SRAMCAN_FLS_NBR (28U) /* Max. Filter List Standard Number      */
#define SRAMCAN_FLE_NBR (8U)  /* Max. Filter List Extended Number      */
#define SRAMCAN_RF0_NBR (3U)  /* RX FIFO 0 Elements Number             */
#define SRAMCAN_RF1_NBR (3U)  /* RX FIFO 1 Elements Number             */
#define SRAMCAN_TEF_NBR (3U)  /* TX Event FIFO Elements Number         */
#define SRAMCAN_TFQ_NBR (3U)  /* TX FIFO/Queue Elements Number         */

#define SRAMCAN_FLS_SIZE (1U * 4U)  /* Filter Standard Element Size in bytes */
#define SRAMCAN_FLE_SIZE (2U * 4U)  /* Filter Extended Element Size in bytes */
#define SRAMCAN_RF0_SIZE (18U * 4U) /* RX FIFO 0 Elements Size in bytes      */
#define SRAMCAN_RF1_SIZE (18U * 4U) /* RX FIFO 1 Elements Size in bytes      */
#define SRAMCAN_TEF_SIZE (2U * 4U)  /* TX Event FIFO Elements Size in bytes  */
#define SRAMCAN_TFQ_SIZE (18U * 4U) /* TX FIFO/Queue Elements Size in bytes  */

#define SRAMCAN_FLSSA ((uint32_t)0) /* Filter List Standard Start Address */
#define SRAMCAN_FLESA ((uint32_t)(SRAMCAN_FLSSA + (SRAMCAN_FLS_NBR * SRAMCAN_FLS_SIZE)))
#define SRAMCAN_RF0SA ((uint32_t)(SRAMCAN_FLESA + (SRAMCAN_FLE_NBR * SRAMCAN_FLE_SIZE)))
#define SRAMCAN_RF1SA ((uint32_t)(SRAMCAN_RF0SA + (SRAMCAN_RF0_NBR * SRAMCAN_RF0_SIZE)))
#define SRAMCAN_TEFSA ((uint32_t)(SRAMCAN_RF1SA + (SRAMCAN_RF1_NBR * SRAMCAN_RF1_SIZE)))
#define SRAMCAN_TFQSA ((uint32_t)(SRAMCAN_TEFSA + (SRAMCAN_TEF_NBR * SRAMCAN_TEF_SIZE)))
#define SRAMCAN_SIZE  ((uint32_t)(SRAMCAN_TFQSA + (SRAMCAN_TFQ_NBR * SRAMCAN_TFQ_SIZE)))

// --- Internal helper API, implemented in fdcan_priv.c ---
// None of this is part of the public surface (fdcan.h); it exists purely
// to keep fdcan.c free of register/bit-level detail.

/// Base address of fdcan's slice of shared message RAM.
uint32_t PHAL_FDCAN_priv_ramBase(FDCAN_GlobalTypeDef *fdcan);

/// Enable FDCAN kernel + peripheral clocks (PCLK1-sourced).
void PHAL_FDCAN_priv_enableClock(void);

/// Enter INIT mode and unlock configuration registers (CCE). Blocks until entered.
void PHAL_FDCAN_priv_enterConfig(FDCAN_GlobalTypeDef *fdcan);

/// Leave INIT mode, ending configuration. Blocks until left.
void PHAL_FDCAN_priv_exitConfig(FDCAN_GlobalTypeDef *fdcan);

/**
 * Look up the NBTP register value for one of the bit rates supported by
 * PHAL_FDCAN_init.
 * @return true and writes *nbtp on success; false if bit_rate is unsupported.
 */
bool PHAL_FDCAN_priv_getNBTP(uint32_t bit_rate, uint32_t *nbtp);

/// Program `num_sid` exact-match standard filters, all routed to RX FIFO0.
void PHAL_FDCAN_priv_writeStandardFilters(FDCAN_GlobalTypeDef *fdcan, uint32_t *sid_list, uint32_t num_sid);

/// Program `num_xid` exact-match extended filters, all routed to RX FIFO0.
void PHAL_FDCAN_priv_writeExtendedFilters(FDCAN_GlobalTypeDef *fdcan, uint32_t *xid_list, uint32_t num_xid);

/// Write one classic-CAN TX element for `msg` into the next free TX FIFO slot.
void PHAL_FDCAN_priv_writeTxElement(FDCAN_GlobalTypeDef *fdcan, CanMsgTypeDef_t *msg);

/**
 * Pop and decode the oldest element from RX FIFO0.
 * @return true and fills *msg on success; false if RX FIFO0 is empty.
 */
bool PHAL_FDCAN_priv_readRxElement(FDCAN_GlobalTypeDef *fdcan, CanMsgTypeDef_t *msg);

#endif // __PHAL_G4_FDCAN_PRIV_H__