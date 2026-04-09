/**
 * @file flash.h
 * @brief G4 Flash HAL - page-based flash operations
 * @version 0.1
 *
 * STM32G4 uses 2 KB pages, double-word (64-bit) programming.
 */

#ifndef _PHAL_G4_FLASH_H_
#define _PHAL_G4_FLASH_H_

#include "common/phal_G4/phal_G4.h"

enum {
    FLASH_OK = 0,
    FLASH_FAIL,
    FLASH_ADDR_NOT_CLEARED,
    FLASH_TIMEOUT,
};

#define FLASH_KEY_1 0x45670123
#define FLASH_KEY_2 0xCDEF89AB

#define PHAL_FLASH_TIMEOUT 500000U

#ifdef STM32G474xx
#define FLASH_PAGE_SIZE      ((uint32_t)2048)
#define FLASH_TOTAL_PAGES    ((uint32_t)256)
#define FLASH_BASE_ADDR      ((uint32_t)0x08000000)
#define FLASH_END_ADDR       ((uint32_t)0x0807FFFF)
#endif

uint32_t PHAL_flashReadU32(uint32_t addr);

uint8_t PHAL_flashWriteU32(uint32_t address, uint32_t value);
uint8_t PHAL_flashWriteU64(uint32_t address, uint64_t data);
uint8_t PHAL_flashWriteU32_Buffered(uint32_t address, uint32_t* data, uint32_t count);

uint8_t PHAL_flashErasePage(uint16_t page);
uint8_t PHAL_flashErase(uint32_t* addr, uint32_t words);

#endif
