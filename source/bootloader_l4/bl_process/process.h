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

/**
 * @brief Process an incoming bootlaoder command
 * 
 * @param cmd 
 * @param data 
 */
void process_bl_cmd(BLCmd_t cmd, uint64_t data);

#endif