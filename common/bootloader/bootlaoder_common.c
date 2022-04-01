/**
 * @file bootlaoder_common.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2022-03-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "bootloader_common.h"
#include "stm32l4xx.h"

extern BootlaoderSharedMemory_t bootloader_shared_memory;

int Bootloader_ResetForFirmwareDownload()
{
    bootloader_shared_memory.magic_word     = BOOTLOADER_SHARED_MEMORY_MAGIC;
    bootloader_shared_memory.reset_reason   = RESET_REASON_DOWNLOAD_FW;
    bootloader_shared_memory.reset_count    = 0;
    NVIC_SystemReset();
}

int Bootloader_ResetForWatchdog()
{
    bootloader_shared_memory.magic_word     = BOOTLOADER_SHARED_MEMORY_MAGIC;
    bootloader_shared_memory.reset_reason   = RESET_REASON_WATCHDOG;
    bootloader_shared_memory.reset_count    = 0;
    NVIC_SystemReset();
}