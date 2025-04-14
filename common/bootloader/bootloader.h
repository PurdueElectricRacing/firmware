/**
 * @file bootloader.h
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief CAN Bootloader:
 *        - A/B double bank flash buffer + CRC checksum
 *        - Download/Upload firmware over buffered CAN-TP (WIP)
 *        - Load/store backup firmware
 * @version 0.1
 * @date 2024-12-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __BOOTLOADER_COMMON_H__
#define __BOOTLOADER_COMMON_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(STM32F407xx)
#include "stm32f4xx.h"
#include "stm32f407xx.h"
#elif defined(STM32F732xx)
#include "stm32f7xx.h"
#include "stm32f732xx.h"
#else
#error "Please define a MCU arch"
#endif

#if defined(STM32F407xx)
/* F4:
 * 0x08000000 ]  16K [Bootloader code]
 * 0x08004000 ]  16K [Metadata A]
 * 0x08008000 ]  16K [Metadata B]
 * 0x08040000 ] 256K [Bank A: Application]
 * 0x08080000 ] 256K [Bank B: Buffer]
 * 0x080c0000 ] 256K [Bank C: Backup firmware]
 */
#define MAX_FIRMWARE_SIZE        0x40000 // 256K
#define BL_ADDRESS_BOOTLOADER 0x08000000 // 0: Bootloader (16K, sector 0)
#define BL_ADDRESS_METADATA   0x08004000 // 1: Metadata (16K, sector 1)
#define BL_ADDRESS_META_C     0x08008000 // 1: Metadata (16K, sector 2)
#define BL_ADDRESS_BANK_A     0x08040000 // 2: Bank A: Application (256K, sector 6..7)
#define BL_ADDRESS_BANK_B     0x08080000 // 3: Bank B: Temporary buffer (256K, sector 8..9)
#define BL_ADDRESS_BANK_C     0x080c0000 // 4: Bank C: Backup firmware (256K, sector 10..11)

#elif defined(STM32F732xx)
/* F7:
 * 0x08000000 ]  16K [Bootloader code]
 * 0x08004000 ]  16K [Metadata A]
 * 0x08008000 ]  16K [Metadata B]
 * 0x08020000 ] 128K [Bank A: Application]
 * 0x08040000 ] 128K [Bank B: Buffer]
 * 0x08060000 ] 128K [C: Backup firmware]
 */
#define MAX_FIRMWARE_SIZE        0x20000 // 128K
#define BL_ADDRESS_BOOTLOADER 0x08000000 // 0: Bootloader (16K, sector 0)
#define BL_ADDRESS_METADATA   0x08004000 // 1: Metadata (16K, sector 1)
#define BL_ADDRESS_META_C     0x08008000 // 1: Metadata (16K, sector 2)
#define BL_ADDRESS_BANK_A     0x08020000 // 2: Bank A: Application (128K, sector 5)
#define BL_ADDRESS_BANK_B     0x08040000 // 3: Bank B: Temporary buffer (128K, sector 6)
#define BL_ADDRESS_BANK_C     0x08060000 // 4: Bank C: Backup firmware (128K, sector 10..11)

#else
#endif

#define BL_METADATA_MAGIC  0xFEE1DEAD
// NOR Flash so bits flip from 0b1111 -> 0b0000
#define BL_FIRMWARE_VERIFIED     (0x00000000)
#define BL_FIRMWARE_NOT_VERIFIED (0xffffffff)

/* CAN Message structure for UDS VAR */
typedef struct {
    uint32_t magic; // magic number to verify bootloader exists
    uint32_t addr;  // address of the bank (A/B/Backup)
    uint32_t words; // words (u32) in firmware
    uint32_t crc;   // crc of the firmware
    uint32_t flags; // unused, potentially checksum of meta itself
    uint32_t verified; // bitflip to 1 if verified during application
} __attribute__((__packed__, aligned(sizeof(uint32_t)))) bl_metadata_t;

#define BL_METADATA_WC ((sizeof(bl_metadata_t)) / (sizeof(uint32_t)))
#define BL_METADATA_VERIFIED_ADDR ((BL_ADDRESS_METADATA) + (((BL_METADATA_WC) - 1) * sizeof(uint32_t))) // last member
static_assert(sizeof(bl_metadata_t) == sizeof(uint32_t) * BL_METADATA_WC);
static_assert((BL_METADATA_VERIFIED_ADDR) == 0x08004014);

bool BL_isBootloaderLoaded(void);
bool BL_metaSanityCheck(bl_metadata_t *meta);
void BL_markFirmwareVerified(void);
bool BL_processCommand(uint8_t cmd, uint64_t data);
bool BL_setMetadata(uint32_t addr, uint32_t words, uint32_t crc);
bool BL_memcpyFlashBuffer(uint32_t addr_dst, uint32_t addr_src, uint32_t words, uint32_t crc);
void _BL_sendStatusMessage(uint8_t cmd, uint8_t err, uint32_t data);

#define BL_sendStatusError(cmd, err, data) (_BL_sendStatusMessage(cmd, err, data))
#define BL_sendStatusGood(cmd, data) (_BL_sendStatusMessage(cmd, BLERROR_NONE, data))

typedef enum __attribute__ ((__packed__))
{
    BLERROR_NONE        = 0,
    BLERROR_CRC         = 1,
    BLERROR_FLASH       = 2,
    BLERROR_SIZE        = 3,
    BLERROR_META        = 4,
    BLERROR_UNKNOWN     = 5,
} BLError_t;
static_assert(sizeof(BLError_t) == sizeof(uint8_t));

typedef enum __attribute__ ((__packed__))
{
    BLCMD_PING          = 0,
    BLCMD_START         = 1,
    BLCMD_CRC           = 2,
    BLCMD_DATA          = 3,
    BLCMD_STAT          = 8,
    BLCMD_BACKUP        = 9,
} BLCmd_t;
static_assert(sizeof(BLCmd_t) == sizeof(uint8_t));

#define UDS_CMD_BL_PING       0x00
#define UDS_CMD_BL_START      0x01
#define UDS_CMD_BL_CRC        0x02
#define UDS_CMD_BL_DATA       0x03
#define UDS_CMD_BL_STAT       0x08
#define UDS_CMD_BL_BACKUP     0x09

#endif // __BOOTLOADER_COMMON_H__
