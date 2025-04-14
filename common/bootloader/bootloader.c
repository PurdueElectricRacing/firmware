/**
 * @file bootloader.c
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief CAN Bootloader:
 *  - A/B partition (seamless) updates for OTA
 *  - Load/store locked backup firmware in partition C
 *  - Download/Upload firmware over buffered CAN-TP (WIP, kinda)
 *
 * @version 0.1
 * @date 2024-12-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#if defined(STM32L496xx) || defined(STM32L432xx)
#include "common/phal_L4/flash/flash.h"
#endif
#if defined(STM32F407xx) || defined(STM32F732xx)
#include "common/phal_F4_F7/flash/flash.h"
#include "common/phal_F4_F7/crc/crc.h"
#endif

#include "common/uds/uds.h"
#include "bootloader.h"

static bool bl_cmd_inprogress = false;
static bool bl_flash_inprogress = false;
static uint32_t firmware_wc_total = 0;
extern char _eboot_flash;

void _BL_sendStatusMessage(uint8_t cmd, uint8_t err, uint32_t data)
{
    // TODO classify status
    uint64_t payload = (uint64_t)cmd & 0xff | (uint64_t)(err & 0xff) << 8 | ((uint64_t)data) << 16;
    udsFrameSend(payload);
}

bool BL_isBootloaderLoaded(void)
{
    // Check whether we currently have bootloader flashed at all
    // or if we just have the raw application starting at start of flash
    // If there's no bootloader we'd be overwriting the app's own code
    // when we bit flip the verified firmware in the metadata section
    uint32_t app_start = (uint32_t)((void *) &_eboot_flash);
    return (app_start == BL_ADDRESS_BANK_A); // linker hack
}

bool BL_metaSanityCheck(bl_metadata_t *meta)
{
    // TODO compute CRC of meta itself
    return ((meta->magic == BL_METADATA_MAGIC) && meta->crc && meta->words && ((meta->words << 2) < MAX_FIRMWARE_SIZE) && (meta->addr >= FLASH_BASE) && (meta->addr <= FLASH_END));
}

void BL_markFirmwareVerified(void)
{
    if (BL_isBootloaderLoaded())
    {
        bl_metadata_t meta;
        PHAL_flashReadU32_Buffered(BL_ADDRESS_METADATA, (uint32_t)&meta, BL_METADATA_WC);
        if ((meta.verified == BL_FIRMWARE_NOT_VERIFIED) && BL_metaSanityCheck(&meta))
        {
            // Flip bits
            PHAL_flashWriteU32(BL_METADATA_VERIFIED_ADDR, BL_FIRMWARE_VERIFIED);
        }
    }
}

static bool BL_processCommand_Start(uint64_t data)
{
    uint32_t words = (data & 0xffff);
    uint32_t flags = (data >> 16) & 0xffff; // unused

    firmware_wc_total = 0;

    // TODO store CRC + size at the end of firmware
    if (!words || ((words << 2) >= MAX_FIRMWARE_SIZE))
    {
        BL_sendStatusError(BLCMD_START, BLERROR_SIZE, words);
        return false;
    }

    if (PHAL_flashErase((uint32_t *)BL_ADDRESS_BANK_B, words) != FLASH_OK)
    {
        BL_sendStatusError(BLCMD_START, BLERROR_FLASH, words);
        return false;
    }

    firmware_wc_total = words;
    bl_flash_inprogress = true;
    BL_sendStatusGood(BLCMD_START, firmware_wc_total);
    return true;
}

#if 0
// TODO bandwidth is halved because we send u32s with a u16 index.
// Integrate ISO-TP branch and properly buffer flash writes
void bitstream_data_CALLBACK(CanParsedData_t* msg_data_a)
{
    uint64_t data = *((uint64_t *) msg_data_a->raw_data);
    // firmware max 0x40000, so max ID is 0xffff, u16 sufficient
    uint32_t index = data & 0xffff;
    uint32_t payload = (data >> 16) & 0xffffffff;

#if 1
    uint32_t buffer_addr = BL_ADDRESS_BANK_B + (uint32_t)index * sizeof(uint32_t) - (0 * sizeof(buffer[0]));
    if (PHAL_flashWriteU32_Buffered(buffer_addr, &payload, 1) != FLASH_OK)
    {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
    }
#else // flash buffering (works but kinda unsafe)
    buffer[buffer_index++] = payload;
    if (buffer_index == BUFFER_SIZE || index == (firmware_size_total / 4 - 1))
    {
        uint32_t buffer_addr = BL_ADDRESS_BANK_B + (uint32_t)index * sizeof(uint32_t) - ((buffer_index - 1) * sizeof(buffer[0]));
        if (PHAL_flashWriteU32_Buffered(buffer_addr, buffer, buffer_index) != FLASH_OK)
        {
            BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        }
        buffer_index = 0;
    }
#endif

    BL_sendStatusMessage(BLSTAT_VALID, 6); // dont send ack msg, too slow
    return;
}
#else
void BL_processCommand_Data(uint64_t data)
{
    uint32_t index = data & 0xffff;
    uint32_t payload = (data >> 16) & 0xffffffff;

    uint32_t buffer_addr = BL_ADDRESS_BANK_B + index * sizeof(uint32_t);
    if (PHAL_flashWriteU32(buffer_addr, payload) != FLASH_OK)
    {
        BL_sendStatusError(BLCMD_DATA, BLERROR_FLASH, payload);
    }
    BL_sendStatusGood(BLCMD_DATA, 6); // dont send ack msg, too slow
}
#endif

static bool _BL_setMetadata(uint32_t addr, uint32_t words, uint32_t crc, uint32_t flags, bool bank_c)
{
    bl_metadata_t meta = {
        .magic = BL_METADATA_MAGIC,
        .addr = addr,
        .words = words,
        .crc = crc,
        .flags = flags,
        .verified = BL_FIRMWARE_NOT_VERIFIED,
    };
    uint32_t meta_addr = (!bank_c) ? BL_ADDRESS_METADATA : BL_ADDRESS_META_C;

    if (PHAL_flashErase((uint32_t *)meta_addr, BL_METADATA_WC) != FLASH_OK)
    {
        BL_sendStatusError(BLCMD_CRC, BLERROR_FLASH, BL_METADATA_WC);
        return false;
    }
    if (PHAL_flashWriteU32_Buffered(meta_addr, (uint32_t *)&meta, BL_METADATA_WC) != FLASH_OK)
    {
        BL_sendStatusError(BLCMD_CRC, BLERROR_FLASH, BL_METADATA_WC);
        return false;
    }

    return true;
}

bool BL_setMetadata(uint32_t addr, uint32_t words, uint32_t crc)
{
    return _BL_setMetadata(addr, words, crc, 0, false);
}

static bool BL_setMetadataC(uint32_t addr, uint32_t words, uint32_t crc)
{
    return _BL_setMetadata(addr, words, crc, 0, true);
}

bool BL_memcpyFlashBuffer(uint32_t addr_dst, uint32_t addr_src, uint32_t words, uint32_t crc)
{
    if (PHAL_flashErase((uint32_t *)addr_dst, words) != FLASH_OK)
        return false;

    for (uint32_t i = 0; i < words; i++)
    {
        uint32_t offset = i * sizeof(uint32_t);
        if (PHAL_flashWriteU32(addr_dst + offset, *(__IO uint32_t*)(addr_src + offset)) != FLASH_OK)
            return false;
    }

    return PHAL_CRC32_Calculate((uint32_t *)addr_dst, words) == crc;
}

static bool BL_processCommand_CRC(uint64_t data)
{
    uint32_t crc_app = data & 0xffffffff;
    uint32_t flags = (data >> 32) & 0xffff;

    uint32_t words = firmware_wc_total;
    if (!words)
    {
        BL_sendStatusError(BLCMD_CRC, BLERROR_SIZE, words);
        return false;
    }

    uint32_t crc_flash = PHAL_CRC32_Calculate((uint32_t *)BL_ADDRESS_BANK_B, words);
    if (crc_flash != crc_app)
    {
        BL_sendStatusError(BLCMD_CRC, BLERROR_CRC, crc_flash);
        return false;
    }

    if (flags == 0x1111) // set backup
    {
        if (BL_memcpyFlashBuffer(BL_ADDRESS_BANK_C, BL_ADDRESS_BANK_B, words, crc_flash) &&
            BL_setMetadataC(BL_ADDRESS_BANK_C, words, crc_flash)) // TODO CRC of meta itself
        {
            BL_sendStatusGood(BLCMD_CRC, crc_flash);
            bl_flash_inprogress = false;
            return true;
        }
        BL_sendStatusError(BLCMD_CRC, BLERROR_FLASH, words);
        return false;
    }

    if (!BL_setMetadata(BL_ADDRESS_BANK_B, words, crc_app))
    {
        BL_sendStatusError(BLCMD_CRC, BLERROR_FLASH, words);
        return false;
    }

    BL_sendStatusGood(BLCMD_CRC, crc_flash);
    bl_flash_inprogress = false;

    return true;
}

static void BL_processCommand_Ping(uint64_t data)
{
    BL_sendStatusGood(BLCMD_PING, BL_METADATA_MAGIC);
}

static void BL_processCommand_Stat(uint64_t data)
{
    bl_metadata_t meta;
    PHAL_flashReadU32_Buffered(BL_ADDRESS_METADATA, (uint32_t)&meta, BL_METADATA_WC);
    BL_sendStatusGood(BLCMD_STAT, meta.addr);
    BL_sendStatusGood(BLCMD_STAT, meta.words);
    BL_sendStatusGood(BLCMD_STAT, meta.crc);
    BL_sendStatusGood(BLCMD_STAT, meta.verified);

    PHAL_flashReadU32_Buffered(BL_ADDRESS_META_C, (uint32_t)&meta, BL_METADATA_WC);
    BL_sendStatusGood(BLCMD_STAT, meta.addr);
    BL_sendStatusGood(BLCMD_STAT, meta.words);
    BL_sendStatusGood(BLCMD_STAT, meta.crc);
    BL_sendStatusGood(BLCMD_STAT, meta.verified);
}

// Running from either bootloader (0) or app (A)
// Copy C to B, configure to boot from B on next boot
static bool BL_processCommand_LoadBackup(uint64_t data)
{
    bl_metadata_t meta;
    PHAL_flashReadU32_Buffered(BL_ADDRESS_META_C, (uint32_t)&meta, BL_METADATA_WC);
    if (BL_metaSanityCheck(&meta) &&
        BL_memcpyFlashBuffer(BL_ADDRESS_BANK_B, BL_ADDRESS_BANK_C, meta.words, meta.crc) &&
        BL_setMetadata(BL_ADDRESS_BANK_B, meta.words, meta.crc))
    {
        BL_sendStatusGood(BLCMD_BACKUP, meta.crc);
        return true;
    }

    BL_sendStatusError(BLCMD_BACKUP, BLERROR_FLASH, 0);
    return false;
}

bool BL_processCommand(uint8_t cmd, uint64_t data)
{
    bl_cmd_inprogress = true;

    switch (cmd)
    {
        case UDS_CMD_BL_PING:
            BL_processCommand_Ping(data);
            break;
        case UDS_CMD_BL_START:
            BL_processCommand_Start(data);
            break;
        case UDS_CMD_BL_DATA:
            BL_processCommand_Data(data);
            break;
        case UDS_CMD_BL_CRC:
            BL_processCommand_CRC(data);
            break;
        case UDS_CMD_BL_STAT:
            BL_processCommand_Stat(data);
            break;
        case UDS_CMD_BL_BACKUP:
            BL_processCommand_LoadBackup(data);
            break;
    }

    bl_cmd_inprogress = false;

    return true;
}

bool BL_flashStarted(void)
{
    return bl_cmd_inprogress || bl_flash_inprogress;
}
