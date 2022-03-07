/**
 * @file process.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _PROCESS_H
#define _PROCESS_H

#include "inttypes.h"
#include "stdbool.h"
#include "can_parse.h"
#include "node_defs.h"

typedef enum
{
    BLCMD_INFO = 0x1,
    BLCMD_ADD_ADDRESS = 0x2,
    BLCMD_FW_DATA = 0x3
} BLCmd_t;

typedef enum
{
    BLSTAT_INVALID = 0,
    BLSTAT_BOOT = 1,        /* Bootloader boot alert */
    BLSTAT_WAIT = 2,        /* Waiting to get BLCMD */
    BLSTAT_PROGRESS = 3,    /* Progress update for bootlaoder download */
    BLSTAT_DONE = 4,        /* Completed the application download */
    BLSTAT_JUMP_TO_APP = 5, /* About to jump to application */
    BLSTAT_INVAID_APP = 6,  /* Did not attempt to boot because the starting address was invalid */
    BLSTAT_UNKOWN_CMD = 7,  /* Incorrect CAN command message format */
} BLStatus_t;

void BL_init(uint32_t* app_flash_start, volatile uint32_t* timeout_ticks_ptr);

/**
 * @brief Process an incoming bootlaoder command
 * 
 * @param cmd 
 * @param data 
 */
void BL_processCommand(BLCmd_t cmd, uint32_t data);

/**
 * @brief The entire application has been written to flash.
 * 
 * @return true 
 * @return false 
 */
bool BL_flashComplete(void);

/**
 * @brief Send a Bootloader status message with the correct app ID
 * 
 * @param cmd  Command/status enum, see BLStatus_t
 * @param data Status data, context specific
 */
void BL_sendStatusMessage(uint8_t cmd, uint32_t data);


#endif