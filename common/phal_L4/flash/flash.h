/**
 * @file hal_flash.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef PER_HAL_FLASH
#define PER_HAL_FLASH

#include "stm32l4xx.h"

enum {
    FLASH_OK = 0,
    FLASH_FAIL,
    FLASH_ADDR_NOT_CLEARED,
    FLASH_TIMEOUT,
};

// Flash magic numbers obtained from family reference manual
#define FLASH_KEY_1 0x45670123
#define FLASH_KEY_2 0xCDEF89AB

#define PHAL_FLASH_TIMEOUT      500000U
#define FLASH_PAGE_SIZE         (0x800) /* Page size in bytes */
#define FLASH_BANK_PG_COUNT     ((FLASH_BANK1_END - FLASH_BASE + 1) / FLASH_PAGE_SIZE) /* Number of pages in a flash bank */
#define USER_FLASH_BASE_ADDRESS (FLASH_BASE) /* First flash memory address */
#define USER_FLASH_END_ADDRESS  (FLASH_END) /* Last usable flash address */
#define MAX_FLASH_SECTOR        (((FLASH_END - FLASH_BASE + 1) / FLASH_PAGE_SIZE) - 1)

/**
 * @brief Writes 64 bits to flash memory
 * 
 * @param address Location to write
 * @param data    Data to write
 */
uint8_t PHAL_flashWriteU64(uint32_t address, uint64_t data);

/**
 * @brief Erase a page in flash
 * 
 * @param page Page to erase
 */
uint8_t PHAL_flashErasePage(uint16_t page);

/**
 * @brief Erase pages containing [addr, addr+words)
 * 
 * @param addr 
 * @param words 
 * @return uint8_t Operation result
 */
uint8_t PHAL_flashErase(uint32_t* addr, uint32_t words);

#endif