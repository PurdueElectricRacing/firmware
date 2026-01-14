#ifndef CAN_COMMON_H
#define CAN_COMMON_H

/**
 * @file can_common.h
 * Common functions and data structures used in every node in the CAN library
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#include <stdint.h>

#include "common/can_library/generated/can_types.h"
#include "common/phal/can.h"
#include "common/queue/queue.h"

typedef struct {
    uint32_t tx_of;      // queue overflow
    uint32_t tx_fail;    // timed out
    uint32_t rx_overrun; // fifo overrun
} can_peripheral_stats_t;

#define CAN_TX_MAILBOX_CNT   (3)
#define CAN_TX_TIMEOUT_MS    (15)
#define CAN_TX_BLOCK_TIMEOUT (30 * 16000)

#define CAN_MAILBOX_HIGH_PRIO 0
#define CAN_MAILBOX_MED_PRIO  1
#define CAN_MAILBOX_LOW_PRIO  2

/* Standard CAN flag and mask definitions */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/* Error class interrupts */
#define CAN_ERR_TX_TIMEOUT 0x00000001U /* TX timeout (not possible in bxCAN) */
#define CAN_ERR_LOSTARB    0x00000002U /* lost arbitration */
#define CAN_ERR_CRTL       0x00000004U /* controller problems */
#define CAN_ERR_PROT       0x00000008U /* protocol violations */
#define CAN_ERR_TRX        0x00000010U /* transceiver status */
#define CAN_ERR_ACK        0x00000020U /* received no ACK on transmission */
#define CAN_ERR_BUSOFF     0x00000040U /* bus off */
#define CAN_ERR_BUSERROR   0x00000080U /* bus error */
#define CAN_ERR_RESTARTED  0x00000100U /* controller restarted */

/* error status of CAN-controller / data[1] */
#define CAN_ERR_CRTL_UNSPEC      0x00 /* unspecified */
#define CAN_ERR_CRTL_RX_OVERFLOW 0x01 /* RX buffer overflow */
#define CAN_ERR_CRTL_TX_OVERFLOW 0x02 /* TX buffer overflow */
#define CAN_ERR_CRTL_RX_WARNING  0x04 /* reached warning level for RX errors */
#define CAN_ERR_CRTL_TX_WARNING  0x08 /* reached warning level for TX errors */
#define CAN_ERR_CRTL_RX_PASSIVE  0x10 /* reached error passive status RX */
#define CAN_ERR_CRTL_TX_PASSIVE  0x20 /* reached error passive status TX */

/* error in CAN protocol (type) / data[2] */
#define CAN_ERR_PROT_UNSPEC   0x00 /* unspecified */
#define CAN_ERR_PROT_BIT      0x01 /* single bit error */
#define CAN_ERR_PROT_FORM     0x02 /* frame format error */
#define CAN_ERR_PROT_STUFF    0x04 /* bit stuffing error */
#define CAN_ERR_PROT_BIT0     0x08 /* unable to send dominant bit */
#define CAN_ERR_PROT_BIT1     0x10 /* unable to send recessive bit */
#define CAN_ERR_PROT_OVERLOAD 0x20 /* bus overload */
#define CAN_ERR_PROT_ACTIVE   0x40 /* active error announcement */
#define CAN_ERR_PROT_TX       0x80 /* error occurred on transmission */

/* error in CAN protocol (location) / data[3] */
#define CAN_ERR_PROT_LOC_UNSPEC  0x00 /* unspecified */
#define CAN_ERR_PROT_LOC_SOF     0x03 /* start of frame */
#define CAN_ERR_PROT_LOC_ID28_21 0x02 /* ID bits 28 - 21 (SFF: 10 - 3) */
#define CAN_ERR_PROT_LOC_ID20_18 0x06 /* ID bits 20 - 18 (SFF: 2 - 0 ) */
#define CAN_ERR_PROT_LOC_ID17_13 0x07 /* ID bits 17-13 */
#define CAN_ERR_PROT_LOC_ID12_05 0x0F /* ID bits 12-5 */
#define CAN_ERR_PROT_LOC_ID04_00 0x0E /* ID bits 4-0 */
#define CAN_ERR_PROT_LOC_RTR     0x0C /* RTR bit */
#define CAN_ERR_PROT_LOC_RES1    0x0D /* reserved bit 1 */
#define CAN_ERR_PROT_LOC_RES0    0x09 /* reserved bit 0 */
#define CAN_ERR_PROT_LOC_DLC     0x0B /* data length code */
#define CAN_ERR_PROT_LOC_DATA    0x0A /* data section */
#define CAN_ERR_PROT_LOC_CRC_SEQ 0x08 /* CRC sequence */
#define CAN_ERR_PROT_LOC_CRC_DEL 0x18 /* CRC delimiter */
#define CAN_ERR_PROT_LOC_ACK     0x19 /* ACK slot */
#define CAN_ERR_PROT_LOC_ACK_DEL 0x1B /* ACK delimiter */
#define CAN_ERR_PROT_LOC_EOF     0x1A /* end of frame */
#define CAN_ERR_PROT_LOC_INTERM  0x12 /* intermission */

/* error status of CAN-transceiver / data[4] */
#define CAN_ERR_TRX_UNSPEC             0x00 /* unspecified */
#define CAN_ERR_TRX_CANH_NO_WIRE       0x04 /* CANH: no wire */
#define CAN_ERR_TRX_CANH_SHORT_TO_BAT  0x05 /* CANH: short to BAT */
#define CAN_ERR_TRX_CANH_SHORT_TO_VCC  0x06 /* CANH: short to VCC */
#define CAN_ERR_TRX_CANH_SHORT_TO_GND  0x07 /* CANH: short to GND */
#define CAN_ERR_TRX_CANL_NO_WIRE       0x40 /* CANL: no wire */
#define CAN_ERR_TRX_CANL_SHORT_TO_BAT  0x50 /* CANL: short to BAT */
#define CAN_ERR_TRX_CANL_SHORT_TO_VCC  0x60 /* CANL: short to VCC */
#define CAN_ERR_TRX_CANL_SHORT_TO_GND  0x70 /* CANL: short to GND */
#define CAN_ERR_TRX_CANL_SHORT_TO_CANH 0x80 /* CANL: short to CANH */

#define CAN_ERR_DLC            8
#define CAN_ERR_LOSTARB_UNSPEC 0x00

#ifndef CAN1_IDX
#define CAN1_IDX 0
#endif
#ifndef CAN2_IDX
#define CAN2_IDX 1
#endif

#define GET_PERIPH_IDX(bus) ((bus == CAN1) ? CAN1_IDX : CAN2_IDX)

typedef struct {
    uint32_t rx_of; // queue overflow
    can_peripheral_stats_t can_peripheral_stats[2];
} can_stats_t;

extern can_stats_t can_stats;
extern q_handle_t q_tx_can[][CAN_TX_MAILBOX_CNT];
extern q_handle_t q_rx_can;
extern volatile uint32_t last_can_rx_time_ms;

void CAN_enqueue_tx(CanMsgTypeDef_t *msg);

#include "common/can_library/generated/can_router.h"

void CAN_tx_update();
void CAN_rx_update();
void CAN_handle_irq(CAN_TypeDef *bus, uint8_t fifo);
bool CAN_library_init();

#endif
