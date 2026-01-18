#ifndef __PHAL_G4_FDCAN_H__
#define __PHAL_G4_FDCAN_H__

#include "common/phal_G4/phal_G4.h"

typedef struct
{
    FDCAN_GlobalTypeDef* Bus; /*!< Specifies the bus. */
    uint16_t StdId; /*!< Specifies the standard identifier. */
    uint32_t ExtId; /*!< Specifies the extended identifier. */
    uint32_t IDE; /*!< Specifies the type of identifier for the message that will be transmitted.  */
    uint32_t DLC; /*!< Specifies the length of the frame that will be transmitted. */
    uint8_t Data[8]; /*!< Contains the data to be transmitted. */
} CanMsgTypeDef_t;

bool PHAL_FDCAN_init(FDCAN_GlobalTypeDef* fdcan, bool test_mode, uint32_t bit_rate);
void PHAL_FDCAN_setFilters(FDCAN_GlobalTypeDef* fdcan, uint32_t* sid_list, uint32_t num_sid, uint32_t* xid_list, uint32_t num_xid);
void PHAL_FDCAN_send(CanMsgTypeDef_t* msg);
void __attribute__((weak)) PHAL_FDCAN_rxCallback(CanMsgTypeDef_t* msg);

#define MAX_NUM_XID_FILTER (8)
#define MAX_NUM_SID_FILTER (28)

#define AF_NUM_FDCAN1           (9)
#define AF_NUM_FDCAN2           (9)
#define AF_NUM_FDCAN13          (11)

#define GPIO_INIT_FDCAN1RX_PA11 GPIO_INIT_AF(GPIOA, 11, AF_NUM_FDCAN1, GPIO_OUTPUT_ULTRA_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN1TX_PA12 GPIO_INIT_AF(GPIOA, 12, AF_NUM_FDCAN1, GPIO_OUTPUT_ULTRA_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN)

#define GPIO_INIT_FDCAN2RX_PB12 GPIO_INIT_AF(GPIOB, 12, AF_NUM_FDCAN2, GPIO_OUTPUT_ULTRA_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN2TX_PB13 GPIO_INIT_AF(GPIOB, 13, AF_NUM_FDCAN2, GPIO_OUTPUT_ULTRA_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN)

#define GPIO_INIT_FDCAN3RX_PA8 GPIO_INIT_AF(GPIOA, 8, AF_NUM_FDCAN13, GPIO_OUTPUT_ULTRA_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN)
#define GPIO_INIT_FDCAN3TX_PB4 GPIO_INIT_AF(GPIOB, 4, AF_NUM_FDCAN13, GPIO_OUTPUT_ULTRA_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN)

#endif // __PHAL_G4_FDCAN_H__
