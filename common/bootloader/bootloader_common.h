/**
 * @file bootloader_common.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Common bootloader functions for all firmware components. 
 * Useful for entering the bootloader to download new firmware
 * @version 0.1
 * @date 2022-03-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _BOOTLOADER_COMMON_H_
#define _BOOTLOADER_COMMON_H_

#include "inttypes.h"
#include "stm32l4xx.h"


#define BOOTLOADER_SHARED_MEMORY_MAGIC (0xABCDBEEF)

typedef enum {
    RESET_REASON_UNKNOWN,
    RESET_REASON_DOWNLOAD_FW,
    RESET_REASON_APP_WATCHDOG,
    RESET_REASON_BL_WATCHDOG,
    RESET_REASON_POR,
    RESET_REASON_BAD_FIRMWARE
} ResetReason_t;

typedef struct {
    uint32_t        magic_word;
    uint32_t        reset_count;
    ResetReason_t   reset_reason;
    uint32_t        application_start_addr;
} BootlaoderSharedMemory_t;

extern BootlaoderSharedMemory_t bootloader_shared_memory;

__attribute__((always_inline)) inline void Bootloader_ConfirmApplicationLaunch()
{
    bootloader_shared_memory.reset_reason   = RESET_REASON_UNKNOWN;
    bootloader_shared_memory.reset_count    = 0;
}

int Bootloader_ResetForFirmwareDownload();
int Bootloader_ResetForWatchdog();

#endif // _BOOTLOADER_COMMON_H_