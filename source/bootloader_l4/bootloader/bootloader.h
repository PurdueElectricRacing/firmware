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

typedef enum
{
    BLCMD_INFO,
    BLCMD_ADD_ADDRESS,
    BLCMD_FW_DATA    ,
} BLCmd_t;

void BL_init(uint32_t app_flash_start);

/**
 * @brief Process an incoming bootlaoder command
 * 
 * @param cmd 
 * @param data 
 */
void BL_processCommand(BLCmd_t cmd, uint64_t data);

#endif