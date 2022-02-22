/**
 * @file bootloader_can.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Common definitions for setting up CAN communication within the bootloader
 * @version 0.1
 * @date 2022-02-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _BOOTLOADER_CAN_H
#define _BOOTLOADER_CAN_H

#if !APP_ID || APP_ID == 1 || APP_ID == 0
    #warning "Please define which device this bootloader will be running on. Defaulting to MAIN_MODULE"
    #define COMPILED_APP_ID APP_MAIN_MODULE
#else 
    #define COMPILED_APP_ID APP_ID
#endif

#define BOOTLOADER_CAN_ADDR_BASE (0xB00B00)
#define BOOTLOADER_CAN_ADDR (BOOTLOADER_CAN_ADDR_BASE | ((COMPILED_APP_ID) & 0xFF))

/**
 * @brief ApplicationID_t
 *    Each registered bootloader application ID lives here.
 * 
 * DO NOT change any existing IDs
 */
typedef enum
{
    APP_MAIN_MODULE  = 0x01,
    APP_DASHBOARD    = 0x02,
    APP_TORQUEVECTOR = 0x03,
    APP_PRECHARGE    = 0x04,
    APP_DRIVELINE_F  = 0x05,
    APP_DRIVELINE_R  = 0x06,
    APP_BMS_A        = 0x07,
    APP_BMS_B        = 0x08,
    APP_BMS_C        = 0x09,
    APP_BMS_D        = 0x0A,
    APP_BMS_E        = 0x0B,
    APP_BMS_F        = 0x0C,
    APP_BMS_G        = 0x0D,
    APP_BMS_H        = 0x0E,
    APP_BMS_I        = 0x0F,
    APP_BMS_J        = 0x10,
    APP_INVALID
} ApplicationID_t;


#endif