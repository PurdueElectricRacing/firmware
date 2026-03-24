/**
 * @file node_defs.h
 * @brief Per-node configuration for bootloader
 */

#ifndef _BOOTLOADER_NODE_DEFS_H_
#define _BOOTLOADER_NODE_DEFS_H_

#include "common/bootloader/bootloader_common.h"

/* App IDs - must match application builds */
#define APP_MAIN_MODULE    0x01
#define APP_DASHBOARD      0x02
#define APP_TORQUE_VECTOR  0x03
#define APP_TORQUEVECTOR   APP_TORQUE_VECTOR
#define APP_A_BOX          0x04
#define APP_PDU            0x05
#define APP_DRIVELINE      0x06
#define APP_G4_TESTING     0x07

/* Per-node configuration based on APP_ID */
#if (APP_ID == APP_MAIN_MODULE)
    #define BL_NODE_NAME "BL_MAIN_MODULE"
    #define BL_USE_FDCAN
    #define BL_FDCAN_PERIPH FDCAN2
    #define BL_FDCAN_BAUD   500000
    /* VCAN: PB12=RX, PB13=TX */
    #define BL_CAN_RX_PORT GPIOB
    #define BL_CAN_RX_PIN  12
    #define BL_CAN_TX_PORT GPIOB
    #define BL_CAN_TX_PIN  13
    /* Status LED */
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    /* CAN message IDs (from canpiler) */
    #include "common/can_library/generated/BL_MAIN_MODULE.h"
    #define BL_CMD_MSG_ID BL_MAIN_MODULE_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_MAIN_MODULE_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_MAIN_MODULE_RESP_MSG_ID
    #define BL_RESP_DLC BL_MAIN_MODULE_RESP_DLC

#elif (APP_ID == APP_DASHBOARD)
    #define BL_NODE_NAME "BL_DASHBOARD"
    #define BL_USE_FDCAN
    #define BL_FDCAN_PERIPH FDCAN2
    #define BL_FDCAN_BAUD   500000
    #define BL_CAN_RX_PORT GPIOB
    #define BL_CAN_RX_PIN  12
    #define BL_CAN_TX_PORT GPIOB
    #define BL_CAN_TX_PIN  13
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    #include "common/can_library/generated/BL_DASHBOARD.h"
    #define BL_CMD_MSG_ID BL_DASHBOARD_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_DASHBOARD_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_DASHBOARD_RESP_MSG_ID
    #define BL_RESP_DLC BL_DASHBOARD_RESP_DLC

#elif (APP_ID == APP_A_BOX)
    #define BL_NODE_NAME "BL_A_BOX"
    #define BL_USE_FDCAN
    #define BL_FDCAN_PERIPH FDCAN2
    #define BL_FDCAN_BAUD   500000
    #define BL_CAN_RX_PORT GPIOB
    #define BL_CAN_RX_PIN  12
    #define BL_CAN_TX_PORT GPIOB
    #define BL_CAN_TX_PIN  13
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    #include "common/can_library/generated/BL_A_BOX.h"
    #define BL_CMD_MSG_ID BL_A_BOX_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_A_BOX_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_A_BOX_RESP_MSG_ID
    #define BL_RESP_DLC BL_A_BOX_RESP_DLC

#elif (APP_ID == APP_TORQUE_VECTOR)
    #define BL_NODE_NAME "BL_TORQUE_VECTOR"
    #define BL_USE_FDCAN
    #define BL_FDCAN_PERIPH FDCAN2
    #define BL_FDCAN_BAUD   500000
    #define BL_CAN_RX_PORT GPIOB
    #define BL_CAN_RX_PIN  12
    #define BL_CAN_TX_PORT GPIOB
    #define BL_CAN_TX_PIN  13
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    #include "common/can_library/generated/BL_TORQUE_VECTOR.h"
    #define BL_CMD_MSG_ID BL_TORQUE_VECTOR_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_TORQUE_VECTOR_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_TORQUE_VECTOR_RESP_MSG_ID
    #define BL_RESP_DLC BL_TORQUE_VECTOR_RESP_DLC

#elif (APP_ID == APP_DRIVELINE)
    #define BL_NODE_NAME "BL_DRIVELINE"
    #define BL_USE_FDCAN
    #define BL_FDCAN_PERIPH FDCAN2
    #define BL_FDCAN_BAUD   500000
    #define BL_CAN_RX_PORT GPIOB
    #define BL_CAN_RX_PIN  12
    #define BL_CAN_TX_PORT GPIOB
    #define BL_CAN_TX_PIN  13
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    #include "common/can_library/generated/BL_DRIVELINE.h"
    #define BL_CMD_MSG_ID BL_DRIVELINE_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_DRIVELINE_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_DRIVELINE_RESP_MSG_ID
    #define BL_RESP_DLC BL_DRIVELINE_RESP_DLC

#elif (APP_ID == APP_G4_TESTING)
    #define BL_NODE_NAME "BL_G4_TESTING"
    #define BL_USE_FDCAN
    #define BL_FDCAN_PERIPH FDCAN2
    #define BL_FDCAN_BAUD   500000
    #define BL_CAN_RX_PORT GPIOB
    #define BL_CAN_RX_PIN  12
    #define BL_CAN_TX_PORT GPIOB
    #define BL_CAN_TX_PIN  13
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    #include "common/can_library/generated/BL_G4_TESTING.h"
    #define BL_CMD_MSG_ID BL_G4_TESTING_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_G4_TESTING_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_G4_TESTING_RESP_MSG_ID
    #define BL_RESP_DLC BL_G4_TESTING_RESP_DLC

#elif (APP_ID == APP_PDU)
    #define BL_NODE_NAME "BL_PDU"
    #define BL_USE_BXCAN
    #define BL_CAN_PERIPH CAN1
    #define BL_CAN_BAUD    500000
    /* PDU uses CAN1: PD0=RX, PD1=TX */
    #define BL_CAN_RX_PORT GPIOD
    #define BL_CAN_RX_PIN  0
    #define BL_CAN_TX_PORT GPIOD
    #define BL_CAN_TX_PIN  1
    #define BL_LED_PORT GPIOC
    #define BL_LED_PIN  13
    #include "common/can_library/generated/BL_PDU.h"
    #define BL_CMD_MSG_ID BL_PDU_CMD_MSG_ID
    #define BL_DATA_MSG_ID BL_PDU_DATA_MSG_ID
    #define BL_RESP_MSG_ID BL_PDU_RESP_MSG_ID
    #define BL_RESP_DLC BL_PDU_RESP_DLC

#else
    #error "APP_ID not defined or unsupported"
#endif

#endif /* _BOOTLOADER_NODE_DEFS_H_ */
