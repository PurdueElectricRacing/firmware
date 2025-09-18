/**
 * @file bootloader.c
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief CAN Bootloader:
 *        - Double bank flash buffer + CRC
 *        - Download/Upload firmware over buffered CAN TP
 *        - Load/store backup firmware
 * @version 0.1
 * @date 2024-11-24
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "bootloader.h"

#include "common/phal/crc.h"
#include "common/phal/flash.h"
#include "common/phal/gpio.h"

#if defined(STM32F407xx)

/* F407 Flash Layout:
 *
 * 0x08000000 ]  16K [bootloader code]
 * 0x08004000 ]  16K [metadata region/boot manager]
 * 0x08008000 ] 256K [application]
 * 0x08040000 ] 256K [temporary buffer]
 * 0x08080000 ] 256K [backup firmware]
 */

#define MAX_FIRMWARE_SIZE     0x40000 // 256 KB
#define BL_ADDRESS_BOOTLOADER 0x08000000 // 0: bootloader (sector 0, 16K)
#define BL_ADDRESS_CRC        0x08004000 // 1: crc metadata (sector 1, 16K)
#define BL_ADDRESS_APP        0x08008000 // 2: application (256K)
#define BL_ADDRESS_BUFFER     0x08040000 // 3: temporary buffer (256K)
#define BL_ADDRESS_BACKUP     0x08080000 // 4: last known good firmware (256K)

#elif defined(STM32F732xx)

/*
 * STM32F732 FLASH LAYOUT (512 KB):
 *
 * 0x08000000 ]  16 KB  [bootloader]
 * 0x08004000 ]  16 KB  [metadata / CRC]
 * 0x08008000 ]  64 KB  [reserved]
 * 0x08010000 ] 128 KB  [application]
 * 0x08030000 ] 128 KB  [temporary buffer]
 * 0x08050000 ] 128 KB  [backup firmware]
 */
#define MAX_FIRMWARE_SIZE 0x20000 // 128 KB

#define BL_ADDRESS_BOOTLOADER 0x08000000 // Sector 0
#define BL_ADDRESS_CRC        0x08004000 // Sector 1
#define BL_ADDRESS_RESERVED   0x08008000 // Sector 2–3 (unused / alignment)
#define BL_ADDRESS_APP        0x08010000 // Sector 4 (128 KB)
#define BL_ADDRESS_BUFFER     0x08030000 // Sector 5 (128 KB)
#define BL_ADDRESS_BACKUP     0x08050000 // Sector 6 (128 KB)

#elif defined(STM32G474xx)

/* G474 Flash Layout (512 KB total, 2 KB pages)
 *
 * 0x08000000 ]  16 KB [bootloader code]       (pages 0–7)
 * 0x08004000 ]  16 KB [metadata region]       (pages 8–15)
 * 0x08008000 ] 160 KB [application]           (pages 16–95)
 * 0x08030000 ] 160 KB [temporary buffer]      (pages 96–175)
 * 0x08058000 ] 160 KB [backup firmware]       (pages 176–255)
 */
#define MAX_FIRMWARE_SIZE     0x28000 // 160 KB
#define BL_ADDRESS_BOOTLOADER 0x08000000 // 0: bootloader (16 KB → pages 0–7)
#define BL_ADDRESS_CRC        0x08004000 // 1: crc metadata (16 KB → pages 8–15)
#define BL_ADDRESS_APP        0x08008000 // 2: application (160 KB → pages 16–95)
#define BL_ADDRESS_BUFFER     0x08030000 // 3: temporary buffer (160 KB → pages 96–175)
#define BL_ADDRESS_BACKUP     0x08058000 // 4: last known good firmware (160 KB → pages 176–255)

#else
#error "Not supported"
#endif

#define BL_ADDRESS_CRC_CRC  ((BL_ADDRESS_CRC) + 0)
#define BL_ADDRESS_CRC_ADDR ((BL_ADDRESS_CRC) + 4)
#define BL_ADDRESS_CRC_SIZE ((BL_ADDRESS_CRC) + 8)

static bool bl_unlock                        = false;
static volatile uint32_t firmware_size_total = 0;

// flash write buffering
#define BUFFER_SIZE 128
static uint32_t buffer[BUFFER_SIZE];
static uint32_t buffer_index = 0;
// #define EXECUTE_FROM_RAM __attribute__ ((section(".RamFunc")))

extern char __isr_vector_start; /* VA of the vector table for the bootloader */
extern char _eboot_flash; /* End of the bootlaoder flash region, same as the application start address */
extern char _estack; /* The start location of the stack */

static void BL_JumptoApplication(void) {
    uint32_t app_reset_handler_address = *(uint32_t*)(((void*)&_eboot_flash + 4));
    uint32_t msp                       = (uint32_t)*((uint32_t*)(((void*)&_eboot_flash)));
    uint32_t estack                    = (uint32_t)((uint32_t*)(((void*)&_estack)));

    // Confirm app exists
    if (app_reset_handler_address == 0xFFFFFFFF || app_reset_handler_address <= BL_ADDRESS_CRC || msp != estack)
        return;

    // Reset all of our used peripherals
    RCC->AHB1RSTR |= RCC_AHB1RSTR_CRCRST;
    RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_CRCRST);
#if defined(STM32F407xx) || defined(STM32F732xx)
    RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
    RCC->APB1RSTR &= ~(RCC_APB1RSTR_CAN1RST);
#else
    RCC->APB1RSTR1 |= RCC_APB1RSTR1_CAN1RST;
    RCC->APB1RSTR1 &= ~(RCC_APB1RSTR1_CAN1RST);
#endif
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // Make sure the interrupts are disabled before we start attempting to jump to the app
    // Getting an interrupt after we set VTOR would be bad.
    // Actually jump to application
    __disable_irq();
    __set_MSP(msp);
    SCB->VTOR = (uint32_t)(uint32_t*)(((void*)&_eboot_flash));
    __enable_irq();
    ((void (*)(void))app_reset_handler_address)(); // jump
}

void BL_checkAndBoot(void) {
    // Check 16K metadata region to decide what to boot
    uint32_t crc_stored = PHAL_flashReadU32(BL_ADDRESS_CRC_CRC);
    uint32_t addr       = PHAL_flashReadU32(BL_ADDRESS_CRC_ADDR);
    uint32_t size       = PHAL_flashReadU32(BL_ADDRESS_CRC_SIZE);
    if (crc_stored && size && (size < MAX_FIRMWARE_SIZE) && (addr >= FLASH_BASE) && (addr <= FLASH_END)) {
        if (PHAL_CRC32_Calculate((uint32_t*)addr, size / 4) == crc_stored) {
            BL_JumptoApplication();
        }
    }
}

static int BL_processCommand_Start(uint32_t size) {
    firmware_size_total = 0;

    // TODO store CRC + size at the end of firmware
    if (!size || size >= MAX_FIRMWARE_SIZE || size & 3) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_SIZE);
        return -1;
    }

    if (PHAL_flashErase((uint32_t*)BL_ADDRESS_BUFFER, size / 4) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        return -1;
    }

    buffer_index        = 0;
    firmware_size_total = size;
    BL_sendStatusMessage(BLSTAT_VALID, firmware_size_total);
    bl_unlock = true;
    return 0;
}

static int BL_memcpy_buffer(uint32_t addr_dst, uint32_t addr_src, uint32_t size) {
    if (PHAL_flashErase((uint32_t*)addr_dst, size / 4) != FLASH_OK)
        return -1;

    for (uint32_t i = 0; i < size / 4; i++) {
        uint32_t offset = i * sizeof(uint32_t);
        if (PHAL_flashWriteU32(addr_dst + offset, *(__IO uint32_t*)(addr_src + offset)) != FLASH_OK)
            return -1;
    }

    return 0;
}

static int BL_setCRCConfig(uint32_t crc, uint32_t addr, uint32_t size) {
    int ret;

    PHAL_flashErase((uint32_t*)BL_ADDRESS_CRC, 3);

    if (PHAL_flashWriteU32(BL_ADDRESS_CRC_CRC, crc) != FLASH_OK) {
        ret = -1;
        goto erase_crc;
    }
    if (PHAL_flashWriteU32(BL_ADDRESS_CRC_ADDR, addr) != FLASH_OK) {
        ret = -1;
        goto erase_crc;
    }
    if (PHAL_flashWriteU32(BL_ADDRESS_CRC_SIZE, size) != FLASH_OK) {
        ret = -1;
        goto erase_crc;
    }

    ret = 0;
    goto exit;

erase_crc:
    // erases whole sector so size doesnt matter
    PHAL_flashErase((uint32_t*)BL_ADDRESS_CRC, 3); // nothing we can do on failure
exit:
    bl_unlock = false;
    return ret;
}

// TODO bandwidth is halved because we send u32s with a u16 index.
// Integrate ISO-TP branch and properly buffer flash writes
void bitstream_data_CALLBACK(CanParsedData_t* msg_data_a) {
    uint64_t data = *((uint64_t*)msg_data_a->raw_data);
    // firmware max 0x40000, so max ID is 0xffff, u16 sufficient
    uint32_t index   = data & 0xffff;
    uint32_t payload = (data >> 16) & 0xffffffff;

#if 1
    uint32_t buffer_addr = BL_ADDRESS_BUFFER + (uint32_t)index * sizeof(uint32_t) - (0 * sizeof(buffer[0]));
    if (PHAL_flashWriteU32_Buffered(buffer_addr, &payload, 1) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
    }
#else // flash buffering (works but kinda unsafe)
    buffer[buffer_index++] = payload;
    if (buffer_index == BUFFER_SIZE || index == (firmware_size_total / 4 - 1)) {
        uint32_t buffer_addr = BL_ADDRESS_BUFFER + (uint32_t)index * sizeof(uint32_t) - ((buffer_index - 1) * sizeof(buffer[0]));
        if (PHAL_flashWriteU32_Buffered(buffer_addr, buffer, buffer_index) != FLASH_OK) {
            BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        }
        buffer_index = 0;
    }
#endif

    //BL_sendStatusMessage(BLSTAT_INVALID, 6); // dont send ack msg, too slow
    return;
}

static int BL_processCommand_CRC(uint32_t crc_app, uint32_t dst_addr) {
    uint32_t size = firmware_size_total;
    if (!size) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_SIZE);
        return -1;
    }

    uint32_t crc_flash;
    crc_flash = PHAL_CRC32_Calculate((uint32_t*)BL_ADDRESS_BUFFER, size / 4);
    if (crc_flash != crc_app) {
        BL_sendStatusMessage(BLSTAT_INVALID_CRC, crc_flash);
        return -1;
    }

#ifdef BL_ENABLE_DOUBLE_BANK
    BL_memcpy_buffer(dst_addr, BL_ADDRESS_BUFFER, size);
    crc_flash = PHAL_CRC32_Calculate((uint32_t*)dst_addr, size / 4);
    if (crc_flash != crc_app) {
        BL_sendStatusMessage(BLSTAT_INVALID_CRC, crc_flash);
        return -1;
    }
#endif // BL_ENABLE_DOUBLE_BANK

    if (BL_setCRCConfig(crc_app, dst_addr, size)) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        return -1;
    }

    BL_sendStatusMessage(BLSTAT_VALID, crc_flash);
    return 0;
}

static void BL_processCommand_Reset(uint32_t data) {
    Bootloader_ResetForFirmwareDownload(); // back to start of flash
}

static void BL_processCommand_Jump(uint32_t data) {
    bl_unlock = true;
    if (data == 0xdeadbeef) // boot backup firmware
    {
        // TODO store backup firmware size somehow
        BL_memcpy_buffer(BL_ADDRESS_APP, BL_ADDRESS_BACKUP, MAX_FIRMWARE_SIZE);
    }
    BL_JumptoApplication();
}

void BL_processCommand(BLCmd_t cmd, uint32_t data) {
    switch (cmd) {
        case BLCMD_START:
            BL_processCommand_Start(data);
            break;
        case BLCMD_CRC:
            BL_processCommand_CRC(data, BL_ADDRESS_APP);
            break;
        case BLCMD_CRC_BACKUP:
            BL_processCommand_CRC(data, BL_ADDRESS_BACKUP);
            break;
        case BLCMD_JUMP:
            BL_processCommand_Jump(data);
            break;
        case BLCMD_RST:
            BL_processCommand_Reset(data);
            break;
        default: {
            BL_sendStatusMessage(BLSTAT_UNKNOWN_CMD, cmd);
            break;
        }
    }
}

bool BL_flashStarted(void) {
    return bl_unlock;
}

/*
 * Component specific callbacks
 */
#define NODE_CASE_BL_RESPONSE(app_id, resp_func) \
    case app_id: \
        resp_func(cmd, data); \
        break;

void BL_sendStatusMessage(uint8_t cmd, uint32_t data) {
    switch (APP_ID) {
        NODE_CASE_BL_RESPONSE(APP_MAIN_MODULE, SEND_MAIN_MODULE_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_DASHBOARD, SEND_DASHBOARD_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_TORQUEVECTOR, SEND_TORQUEVECTOR_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_A_BOX, SEND_A_BOX_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_PDU, SEND_PDU_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_L4_TESTING, SEND_L4_TESTING_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_F4_TESTING, SEND_F4_TESTING_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_F7_TESTING, SEND_F7_TESTING_BL_RESP)
        NODE_CASE_BL_RESPONSE(APP_DAQ, SEND_DAQ_BL_RESP)
        default:
            asm("bkpt");
    }
}

// Quickly setup the CAN callbacks based on Node ID
#define NODE_BL_CMD_CALLBACK(callback_name, can_msg_name, node_id) \
    void callback_name(CanParsedData_t* msg_data_a) { \
        if (APP_ID != node_id) \
            return; \
        BL_processCommand((BLCmd_t)msg_data_a->can_msg_name.cmd, msg_data_a->can_msg_name.data); \
    }

NODE_BL_CMD_CALLBACK(main_module_bl_cmd_CALLBACK, main_module_bl_cmd, APP_MAIN_MODULE)
NODE_BL_CMD_CALLBACK(dashboard_bl_cmd_CALLBACK, dashboard_bl_cmd, APP_DASHBOARD)
NODE_BL_CMD_CALLBACK(torquevector_bl_cmd_CALLBACK, torquevector_bl_cmd, APP_TORQUEVECTOR)
NODE_BL_CMD_CALLBACK(a_box_bl_cmd_CALLBACK, a_box_bl_cmd, APP_A_BOX)
NODE_BL_CMD_CALLBACK(pdu_bl_cmd_CALLBACK, pdu_bl_cmd, APP_PDU)
NODE_BL_CMD_CALLBACK(l4_testing_bl_cmd_CALLBACK, l4_testing_bl_cmd, APP_L4_TESTING)
NODE_BL_CMD_CALLBACK(f4_testing_bl_cmd_CALLBACK, f4_testing_bl_cmd, APP_F4_TESTING)
NODE_BL_CMD_CALLBACK(f7_testing_bl_cmd_CALLBACK, f7_testing_bl_cmd, APP_F7_TESTING)
NODE_BL_CMD_CALLBACK(daq_bl_cmd_CALLBACK, daq_bl_cmd, APP_DAQ)
