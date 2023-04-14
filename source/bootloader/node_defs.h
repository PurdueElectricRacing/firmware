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

/**
 * Each registered bootloader application ID lives here.
 * DO NOT change any existing IDs
 */

#define    APP_MAIN_MODULE     0x01
#define    APP_DASHBOARD       0x02
#define    APP_TORQUEVECTOR    0x03
#define    APP_PRECHARGE       0x04
#define    APP_DRIVELINE_FRONT 0x05
#define    APP_DRIVELINE_REAR  0x06
#define    APP_BMS_A           0x07
#define    APP_BMS_B           0x08
#define    APP_BMS_C           0x09
#define    APP_BMS_D           0x0A
#define    APP_BMS_E           0x0B
#define    APP_BMS_F           0x0C
#define    APP_BMS_G           0x0D
#define    APP_BMS_H           0x0E
#define    APP_BMS_I           0x0F
#define    APP_BMS_J           0x10
#define    APP_L4_TESTING      0x11

#if !defined(APP_ID)
    #warning "Please define which device this bootloader will be running on. Defaulting to APP_MAIN_MODULE"
#endif


/**
 *  Per-board deifintion of the CAN and LED status pins
 */
#if (APP_ID == APP_TORQUEVECTOR) ||\
    ((APP_ID >= APP_BMS_A)  && (APP_ID <= APP_BMS_J))

    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       5
#endif

#if (APP_ID == APP_MAIN_MODULE)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PD0
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PD1

    #define STATUS_LED_GPIO_Port  (GPIOE)
    #define STATUS_LED_Pin        (8)
#endif

#if (APP_ID == APP_DASHBOARD)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PD0
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PD1

    #define STATUS_LED_GPIO_Port (GPIOE) // Precharge LED
    #define STATUS_LED_Pin       (1)
#endif

#if (APP_ID == APP_DRIVELINE_FRONT)  ||\
    (APP_ID == APP_DRIVELINE_REAR)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       3
#endif

#if (APP_ID == APP_L4_TESTING)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOB
    #define STATUS_LED_Pin       3
#endif

#if (APP_ID == APP_PRECHARGE)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

    #define STATUS_LED_GPIO_Port GPIOE
    #define STATUS_LED_Pin       14
#endif

#ifndef CAN_RX_GPIO_CONFIG
    #error "Please provide a board configuration for the compiled application ID"
#endif /* CAN_RX_GPIO_CONFIG */

#endif /* _NODE_DEFS_H */