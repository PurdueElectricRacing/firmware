/**
 * @file bootloader.c
 * @brief Core bootloader logic
 */

#include "bootloader.h"

#include "node_defs.h"

#ifdef STM32G474xx
#include "common/phal_G4/crc/crc.h"
#include "common/phal_G4/fdcan/fdcan.h"
#include "common/phal_G4/flash/flash.h"
#include "common/phal_G4/gpio/gpio.h"
#elif defined(STM32F407xx)
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal_F4_F7/crc/crc.h"
#include "common/phal_F4_F7/flash/flash.h"
#endif

#include <string.h>

/* State */
static bool bl_unlock               = false;
static uint32_t firmware_size_total = 0;
static uint32_t bl_data_index       = 0;
#ifdef STM32G474xx
static uint32_t bl_data_buffer[2]; /* Buffer for 64-bit writes */
static bool bl_data_buffer_full     = false;
static uint32_t bl_data_buffer_addr = 0;

static volatile CanMsgTypeDef_t bl_rx_queue[8];
static volatile uint8_t bl_rx_head = 0;
static volatile uint8_t bl_rx_tail = 0;
#endif

static int BL_flushPendingDataWord(void);
static void BL_handleIncomingCAN(const CanMsgTypeDef_t *msg);
static bool BL_isBootAddressValid(uint32_t addr, uint32_t size);

/* SysTick counter */
volatile uint32_t bootloader_ms = 0;

void SysTick_Handler(void) {
    bootloader_ms++;
}

/**
 * @brief Jump to application
 */
static void BL_JumptoApplication(uint32_t app_base_addr) {
    uintptr_t app_base                 = (uintptr_t)app_base_addr;
    uint32_t app_reset_handler_address = *(uint32_t *)(app_base + 4U);
    uint32_t msp                       = *(uint32_t *)(app_base);

    /* Validate application exists */
    if (app_reset_handler_address == 0xFFFFFFFFU || app_reset_handler_address < BL_FLASH_BASE
        || app_reset_handler_address > BL_FLASH_END
        || app_reset_handler_address <= BL_ADDRESS_CRC) {
        return;
    }

    /* Reset peripherals used by bootloader */
#ifdef STM32G474xx
    RCC->AHB1RSTR |= RCC_AHB1RSTR_CRCRST;
    RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_CRCRST);
    RCC->APB1RSTR1 |= RCC_APB1RSTR1_FDCANRST;
    RCC->APB1RSTR1 &= ~(RCC_APB1RSTR1_FDCANRST);
#elif defined(STM32F407xx)
    RCC->AHB1RSTR |= RCC_AHB1RSTR_CRCRST;
    RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_CRCRST);
    RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
    RCC->APB1RSTR &= ~(RCC_APB1RSTR_CAN1RST);
#endif

    /* Disable SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Perform the jump */
    __disable_irq();
    __set_MSP(msp);
    SCB->VTOR = app_base_addr;
    __enable_irq();
    ((void (*)(void))app_reset_handler_address)();
}

/**
 * @brief Check firmware CRC and boot if valid
 */
void BL_checkAndBoot(void) {
    uint32_t crc_stored = PHAL_flashReadU32(BL_ADDRESS_CRC + BL_CRC_OFFSET_CRC);
    uint32_t addr       = PHAL_flashReadU32(BL_ADDRESS_CRC + BL_CRC_OFFSET_ADDR);
    uint32_t size       = PHAL_flashReadU32(BL_ADDRESS_CRC + BL_CRC_OFFSET_SIZE);

    /* Validate parameters */
    if (crc_stored && BL_isBootAddressValid(addr, size)) {
        /* Calculate CRC over firmware region */
        uint32_t crc_calc = PHAL_CRC32_Calculate((uint32_t *)addr, size / 4);
        if (crc_calc == crc_stored) {
            BL_JumptoApplication(addr);
        }
    }
}

static bool BL_isBootAddressValid(uint32_t addr, uint32_t size) {
    if ((size == 0U) || (size > MAX_FIRMWARE_SIZE) || ((size & 3U) != 0U)) {
        return false;
    }

    if ((addr != BL_ADDRESS_APP) && (addr != BL_ADDRESS_BACKUP)) {
        return false;
    }

    if ((addr < BL_FLASH_BASE) || (addr > BL_FLASH_END)) {
        return false;
    }

    uint32_t end_addr = addr + size - 1U;
    if ((end_addr < addr) || (end_addr > BL_FLASH_END)) {
        return false;
    }

    return true;
}

/**
 * @brief Process START command
 */
static int BL_processCommand_Start(uint32_t size) {
    if (!size || size >= MAX_FIRMWARE_SIZE || (size & 3)) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_SIZE);
        return -1;
    }

    /* Erase buffer region */
    if (PHAL_flashErase((uint32_t *)BL_ADDRESS_BUFFER, size / 4) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        return -1;
    }

    firmware_size_total = size;
    bl_unlock           = true;
    bl_data_index       = 0;
#ifdef STM32G474xx
    bl_data_buffer_full = false;
#endif
    BL_sendStatusMessage(BLSTAT_VALID, firmware_size_total);
    return 0;
}

/**
 * @brief Process CRC command - validate and copy to app region
 */
static int BL_processCommand_CRC(uint32_t crc_app, uint32_t dst_addr) {
    uint32_t size = firmware_size_total;

    if (!size || (size > MAX_FIRMWARE_SIZE)) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_SIZE);
        return -1;
    }

    if (BL_flushPendingDataWord() != 0) {
        return -1;
    }

    /* Calculate CRC of received firmware */
    uint32_t crc_flash = PHAL_CRC32_Calculate((uint32_t *)BL_ADDRESS_BUFFER, size / 4);

    if (crc_flash != crc_app) {
        BL_sendStatusMessage(BLSTAT_INVALID_CRC, crc_flash);
        return -1;
    }

    /* Copy from buffer to destination */
    if (PHAL_flashErase((uint32_t *)dst_addr, size / 4) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        return -1;
    }

    /* Copy word by word */
    for (uint32_t i = 0; i < size / 4; i++) {
        uint32_t word = PHAL_flashReadU32(BL_ADDRESS_BUFFER + i * 4);
        if (PHAL_flashWriteU32(dst_addr + i * 4, word) != FLASH_OK) {
            BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
            return -1;
        }
    }

    /* Write CRC metadata */
    if (PHAL_flashErase((uint32_t *)BL_ADDRESS_CRC, 3) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        return -1;
    }
    if (PHAL_flashWriteU32(BL_ADDRESS_CRC + BL_CRC_OFFSET_CRC, crc_app) != FLASH_OK
        || PHAL_flashWriteU32(BL_ADDRESS_CRC + BL_CRC_OFFSET_ADDR, dst_addr) != FLASH_OK
        || PHAL_flashWriteU32(BL_ADDRESS_CRC + BL_CRC_OFFSET_SIZE, size) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        return -1;
    }

    bl_unlock = false;
    BL_sendStatusMessage(BLSTAT_VALID, crc_flash);
    return 0;
}

/**
 * @brief Process incoming command
 */
void BL_processCommand(uint8_t cmd, uint32_t data) {
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
            BL_checkAndBoot();
            break;
        case BLCMD_RST:
            NVIC_SystemReset();
            break;
        default:
            BL_sendStatusMessage(BLSTAT_UNKNOWN_CMD, cmd);
            break;
    }
}

/**
 * @brief Write firmware data to buffer flash
 * @param index 16-bit word index
 * @param payload 32-bit firmware data
 */
void BL_writeData(uint16_t index, uint32_t payload) {
    if (!bl_unlock) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_LOCKED);
        return;
    }

    uint32_t total_words = firmware_size_total / 4;
    if (index >= total_words) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_ADDR_BOUND);
        return;
    }

    uint32_t addr = BL_ADDRESS_BUFFER + index * 4;

    /* Buffer pairs for 64-bit writes on G4 */
#ifdef STM32G474xx
    if (index & 1) {
        /* Odd index - complete the pair */
        bl_data_buffer[1]    = payload;
        uint64_t double_word = ((uint64_t)bl_data_buffer[1] << 32) | bl_data_buffer[0];
        if (PHAL_flashWriteU64(addr & ~0x7, double_word) != FLASH_OK) {
            BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
            bl_unlock = false;
            return;
        }
        bl_data_buffer_full = false;
    } else {
        /* Even index - start new pair */
        bl_data_buffer_addr = addr & ~0x7U;
        bl_data_buffer[0]   = payload;
        bl_data_buffer_full = true;
    }
#else
    /* F4 can write 32-bit directly */
    if (PHAL_flashWriteU32(addr, payload) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        bl_unlock = false;
        return;
    }
#endif

    if ((uint32_t)index >= bl_data_index) {
        bl_data_index = (uint32_t)index + 1U;
    }
}

/**
 * @brief Check if flash operation is in progress
 */
bool BL_flashStarted(void) {
    return bl_unlock;
}

/**
 * @brief Send status message via CAN
 */
void BL_sendStatusMessage(uint8_t status, uint32_t detail) {
    CanMsgTypeDef_t outgoing = {0};
#ifdef BL_USE_FDCAN
    outgoing.Bus = BL_FDCAN_PERIPH;
#else
    outgoing.Bus = BL_CAN_PERIPH;
#endif
    outgoing.StdId = BL_RESP_MSG_ID;
    outgoing.IDE   = 0;
    outgoing.DLC   = BL_RESP_DLC;

    uint64_t payload = 0;
    payload |= ((uint64_t)(status & 0xFFU)) << 0;
    payload |= ((uint64_t)detail) << 8;
    memcpy(outgoing.Data, &payload, outgoing.DLC);

#ifdef BL_USE_FDCAN
    if (PHAL_FDCAN_txFifoFree(BL_FDCAN_PERIPH)) {
        PHAL_FDCAN_send(&outgoing);
    }
#else
    for (uint8_t mbx = 0; mbx < 3; ++mbx) {
        if (PHAL_txMailboxFree(BL_CAN_PERIPH, mbx)) {
            (void)PHAL_txCANMessage(&outgoing, mbx);
            break;
        }
    }
#endif
}

/**
 * @brief Initialize CAN for bootloader
 */
void BL_CANInit(void) {
#ifdef STM32G474xx
    GPIOInitConfig_t can_pins[] = {
        GPIO_INIT_FDCAN2RX_PB12,
        GPIO_INIT_FDCAN2TX_PB13,
    };

    (void)PHAL_initGPIO(can_pins, sizeof(can_pins) / sizeof(can_pins[0]));
    (void)PHAL_FDCAN_init(BL_FDCAN_PERIPH, false, BL_FDCAN_BAUD);
    uint32_t sid_filters[] = {BL_CMD_MSG_ID, BL_DATA_MSG_ID};
    PHAL_FDCAN_setFilters(BL_FDCAN_PERIPH, sid_filters, 2, NULL, 0);
#elif defined(STM32F407xx)
    GPIOInitConfig_t can_pins[] = {
        GPIO_INIT_CANRX_PD0,
        GPIO_INIT_CANTX_PD1,
    };

    (void)PHAL_initGPIO(can_pins, sizeof(can_pins) / sizeof(can_pins[0]));
    (void)PHAL_initCAN(BL_CAN_PERIPH, false, BL_CAN_BAUD);
#endif
}

/**
 * @brief CAN polling function - process incoming messages
 */
void BL_CANPoll(void) {
#ifdef STM32G474xx
    extern void PHAL_FDCAN_RX_IRQHandler(FDCAN_GlobalTypeDef * fdcan);
    PHAL_FDCAN_RX_IRQHandler(BL_FDCAN_PERIPH);

    while (bl_rx_tail != bl_rx_head) {
        CanMsgTypeDef_t rx = bl_rx_queue[bl_rx_tail];
        bl_rx_tail =
            (uint8_t)((bl_rx_tail + 1U) % (uint8_t)(sizeof(bl_rx_queue) / sizeof(bl_rx_queue[0])));
        BL_handleIncomingCAN(&rx);
    }
#elif defined(STM32F407xx)
    CanMsgTypeDef_t rx = {0};
    while (PHAL_rxCANMessage(BL_CAN_PERIPH, 0, &rx)) {
        BL_handleIncomingCAN(&rx);
    }
    while (PHAL_rxCANMessage(BL_CAN_PERIPH, 1, &rx)) {
        BL_handleIncomingCAN(&rx);
    }
#endif
}

static int BL_flushPendingDataWord(void) {
#ifdef STM32G474xx
    if (!bl_data_buffer_full) {
        return 0;
    }

    uint64_t double_word = ((uint64_t)0xFFFFFFFFU << 32) | bl_data_buffer[0];
    if (PHAL_flashWriteU64(bl_data_buffer_addr, double_word) != FLASH_OK) {
        BL_sendStatusMessage(BLSTAT_INVALID, BLERROR_FLASH);
        bl_unlock = false;
        return -1;
    }
    bl_data_buffer_full = false;
#endif
    return 0;
}

static void BL_handleIncomingCAN(const CanMsgTypeDef_t *msg) {
    if (msg->IDE != 0) {
        return;
    }

    uint64_t payload = 0;
    memcpy(&payload, msg->Data, msg->DLC);

    if (msg->StdId == BL_CMD_MSG_ID) {
        uint8_t cmd   = (uint8_t)((payload >> 0) & 0xFFU);
        uint32_t data = (uint32_t)((payload >> 8) & 0xFFFFFFFFU);
        BL_processCommand(cmd, data);
        return;
    }

    if (msg->StdId == BL_DATA_MSG_ID) {
        uint16_t index = (uint16_t)((payload >> 0) & 0xFFFFU);
        uint32_t data  = (uint32_t)((payload >> 16) & 0xFFFFFFFFU);
        BL_writeData(index, data);
    }
}

#ifdef STM32G474xx
void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg) {
    uint8_t next =
        (uint8_t)((bl_rx_head + 1U) % (uint8_t)(sizeof(bl_rx_queue) / sizeof(bl_rx_queue[0])));
    if (next == bl_rx_tail) {
        return;
    }

    bl_rx_queue[bl_rx_head] = *msg;
    bl_rx_head              = next;
}
#endif
