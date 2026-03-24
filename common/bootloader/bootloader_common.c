/**
 * @file bootloader_common.c
 * @brief Shared bootloader memory and reset functions
 */

#include "bootloader_common.h"

#ifdef STM32G474xx
#include "common/phal_G4/phal_G4.h"
#elif defined(STM32F407xx)
#include "common/phal_F4_F7/phal_F4_F7.h"
#endif

/* Shared memory placed in .noinit section - survives reset */
__attribute__((section(".noinit")))
BootloaderSharedMemory_t bootloader_shared_memory;

void Bootloader_ResetForFirmwareDownload(void) {
    bootloader_shared_memory.magic_word = BOOTLOADER_SHARED_MEMORY_MAGIC;
    bootloader_shared_memory.reset_reason = RESET_REASON_DOWNLOAD_FW;
    bootloader_shared_memory.reset_count = 0;
    NVIC_SystemReset();
}

void Bootloader_ResetForWatchdog(void) {
    bootloader_shared_memory.magic_word = BOOTLOADER_SHARED_MEMORY_MAGIC;
    bootloader_shared_memory.reset_reason = RESET_REASON_APP_WATCHDOG;
    bootloader_shared_memory.reset_count = 0;
    NVIC_SystemReset();
}
