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

// Flash magic numbers obtained from family reference manual
#define FLASH_KEY_1 0x45670123
#define FLASH_KEY_2 0xCDEF89AB

// void PHAL_flashWriteU32(uint32_t* address, uint32_t value);
/**
 * @brief Writes 64 bits to flash memory
 * 
 * @param address Location to write
 * @param data    Data to write
 */
void PHAL_flashWriteU64(uint32_t address, uint64_t data);

/**
 * @brief Erase a page in flash
 * 
 * @param page Page to erase
 */
void PHAL_flashErasePage(uint8_t page);

#endif