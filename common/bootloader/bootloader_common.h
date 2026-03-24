/**
 * @file bootloader_common.h
 * @brief Shared definitions between bootloader and application
 */

#ifndef _BOOTLOADER_COMMON_H_
#define _BOOTLOADER_COMMON_H_

#include <stdint.h>
#include <stdbool.h>

/* Magic word for shared memory validation */
#define BOOTLOADER_SHARED_MEMORY_MAGIC 0xABCDBEEF

/* Reset reasons */
typedef enum {
    RESET_REASON_INVALID = 0,
    RESET_REASON_BUTTON,
    RESET_REASON_DOWNLOAD_FW,
    RESET_REASON_APP_WATCHDOG,
    RESET_REASON_POR,
    RESET_REASON_BAD_FIRMWARE,
} ResetReason_t;

/* Shared memory structure - survives reset via .noinit section */
typedef struct {
    uint32_t magic_word;
    uint32_t reset_count;
    ResetReason_t reset_reason;
} BootloaderSharedMemory_t;

/* G4 Memory Layout */
#ifdef STM32G474xx
#define BL_ADDRESS_CRC       0x08004000
#define BL_ADDRESS_APP       0x08008000
#define BL_ADDRESS_BUFFER    0x08030000
#define BL_ADDRESS_BACKUP    0x08058000
#define BL_FLASH_BASE        0x08000000
#define BL_FLASH_END         0x0807FFFF
#define MAX_FIRMWARE_SIZE    (160 * 1024)
#endif

/* F4 Memory Layout */
#ifdef STM32F407xx
#define BL_ADDRESS_CRC       0x08004000
#define BL_ADDRESS_APP       0x08008000
#define BL_ADDRESS_BUFFER    0x08040000
#define BL_ADDRESS_BACKUP    0x08080000
#define BL_FLASH_BASE        0x08000000
#define BL_FLASH_END         0x080FFFFF
#define MAX_FIRMWARE_SIZE    (256 * 1024)
#endif

/* CRC metadata offsets */
#define BL_CRC_OFFSET_CRC   0x00
#define BL_CRC_OFFSET_ADDR  0x04
#define BL_CRC_OFFSET_SIZE  0x08

/* Bootloader commands */
typedef enum {
    BLCMD_START = 0x1,
    BLCMD_CRC_BACKUP = 0x2,
    BLCMD_CRC = 0x3,
    BLCMD_JUMP = 0x4,
    BLCMD_RST = 0x5,
} BLCmd_t;

/* Bootloader status */
typedef enum {
    BLSTAT_VALID = 0,
    BLSTAT_INVALID = 1,
    BLSTAT_INVALID_CRC = 2,
    BLSTAT_UNKNOWN_CMD = 3,
} BLStatus_t;

/* Bootloader errors */
typedef enum {
    BLERROR_CRC_FAIL = 0,
    BLERROR_LOCKED = 1,
    BLERROR_LOW_ADDR = 2,
    BLERROR_ADDR_BOUND = 3,
    BLERROR_FLASH = 4,
    BLERROR_SIZE = 5,
} BLError_t;

/* Application confirm - call early in main() */
static inline void Bootloader_ConfirmApplicationLaunch(void) {
#if defined(BOOTLOADER_ENABLED)
    extern BootloaderSharedMemory_t bootloader_shared_memory;
    bootloader_shared_memory.reset_reason = RESET_REASON_BUTTON;
#endif
}

/* Request firmware download reset */
void Bootloader_ResetForFirmwareDownload(void);

/* Request watchdog reset */
void Bootloader_ResetForWatchdog(void);

#endif /* _BOOTLOADER_COMMON_H_ */
