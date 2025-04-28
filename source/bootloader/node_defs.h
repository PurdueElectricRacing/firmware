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
 */
#define    APP_A_BOX           0x01
#define    APP_DAQ             0x02
#define    APP_DASHBOARD       0x03
#define    APP_MAIN_MODULE     0x04
#define    APP_PDU             0x05
#define    APP_TORQUEVECTOR    0x06

/**
 *  Per-board deifintion of the CAN and LED status pins
 */
#if (APP_ID == APP_A_BOX)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

#elif (APP_ID == APP_DAQ)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

#elif (APP_ID == APP_DASHBOARD)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PD0
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PD1

#elif (APP_ID == APP_MAIN_MODULE)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

#elif (APP_ID == APP_PDU)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PD0
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PD1

#elif (APP_ID == APP_TORQUEVECTOR)
    #define CAN_RX_GPIO_CONFIG GPIO_INIT_CANRX_PA11
    #define CAN_TX_GPIO_CONFIG GPIO_INIT_CANTX_PA12

#else
    #error "Invalid node ID"
#endif


#ifndef CAN_RX_GPIO_CONFIG
    #error "Please provide a board configuration for the compiled application ID"
#endif /* CAN_RX_GPIO_CONFIG */

#endif /* _NODE_DEFS_H */