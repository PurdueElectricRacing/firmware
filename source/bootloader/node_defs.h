/**
 * @file node_defs.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Common definitions for setting up CAN communication within the bootloader
 * @version 0.1
 * @date 2022-02-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _NODE_DEFS_H
#define _NODE_DEFS_H

#include "common/common_defs/common_defs.h"

/**
 * Each registered bootloader application ID lives here.
 * DO NOT change any existing IDs
 */

#define    APP_MAIN_MODULE     0x01
#define    APP_DASHBOARD       0x02
#define    APP_TORQUEVECTOR    0x03
#define    APP_A_BOX           0x04
#define    APP_PDU             0x05
#define    APP_L4_TESTING      0x06
#define    APP_F4_TESTING      0x07
#define    APP_F7_TESTING      0x08
#define    APP_DAQ             0x09

#if !defined(APP_ID)
    #warning "Please define which device this bootloader will be running on. Defaulting to APP_MAIN_MODULE"
#endif


/**
 *  Per-board deifintion of the CAN and LED status pins
 */
#if (APP_ID == APP_TORQUEVECTOR)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       7

    #define HSI_TRIM_BL_NODE HSI_TRIM_TORQUE_VECTOR
#endif

#if (APP_ID == APP_PDU)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PD0
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PD1

    #define STATUS_LED_GPIO_Port  (GPIOC)
    #define STATUS_LED_Pin        (14)

    #define HSI_TRIM_BL_NODE HSI_TRIM_PDU
#endif

#if (APP_ID == APP_MAIN_MODULE)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port  (GPIOD)
    #define STATUS_LED_Pin        (3)

    #define HSI_TRIM_BL_NODE HSI_TRIM_MAIN_MODULE
#endif

#if (APP_ID == APP_DASHBOARD)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PD0
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PD1

    #define STATUS_LED_GPIO_Port (GPIOE) // Precharge LED
    #define STATUS_LED_Pin       (1)

    #define HSI_TRIM_BL_NODE HSI_TRIM_DASHBOARD
#endif

#if (APP_ID == APP_L4_TESTING)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       3
#endif

#if (APP_ID == APP_F4_TESTING)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       3
#endif

#if (APP_ID == APP_F7_TESTING)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       3
#endif

#if (APP_ID == APP_A_BOX)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOE
    #define STATUS_LED_Pin       14

    #define HSI_TRIM_BL_NODE HSI_TRIM_A_BOX
#endif

#if (APP_ID == APP_DAQ)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOA
    #define STATUS_LED_Pin       1
#endif

#ifndef CAN_RX_GPIO_CONFIG
    #error "Please provide a board configuration for the compiled application ID"
#endif /* CAN_RX_GPIO_CONFIG */

#endif /* _NODE_DEFS_H */