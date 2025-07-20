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
#if defined(STM32F407xx)
#include "stm32f4xx.h"
#elif defined(STM32F732xx)
#include "stm32f7xx.h"
#else
#include "stm32l4xx.h"
#endif

#define BOOTLOADER_SHARED_MEMORY_MAGIC (0xABCDBEEF)

typedef enum {
    BLCMD_START = 0x1, /* Request to start firmware download */
    BLCMD_CRC = 0x3, /* Final CRC-32b check of firmware */
    BLCMD_CRC_BACKUP = 0x2,
    BLCMD_JUMP = 0x4, /* Request to start firmware download */
    BLCMD_RST = 0x5, /* Request for reset */
} BLCmd_t;

typedef enum {
    RESET_REASON_INVALID,
    RESET_REASON_BUTTON,
    RESET_REASON_DOWNLOAD_FW,
    RESET_REASON_APP_WATCHDOG,
    RESET_REASON_POR,
    RESET_REASON_BAD_FIRMWARE
} ResetReason_t;

typedef struct {
    uint32_t magic_word;
    uint32_t reset_count;
    ResetReason_t reset_reason;
} BootlaoderSharedMemory_t;

extern BootlaoderSharedMemory_t bootloader_shared_memory;

__attribute__((always_inline)) inline void Bootloader_ConfirmApplicationLaunch() {
    bootloader_shared_memory.reset_reason = RESET_REASON_BUTTON;
}

int Bootloader_ResetForFirmwareDownload();
int Bootloader_ResetForWatchdog();

#endif // _BOOTLOADER_COMMON_H_