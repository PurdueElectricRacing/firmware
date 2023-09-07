/**
 * @file flash.h
 * @author Chris McGalliard - port of L4 by Adam Busch (busch8@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2023-08-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _PHAL_FLASH_H_
#define _PHAL_FLASH_H_

#if defined(STM32F407xx)
#include "stm32f4xx.h"
#elif defined(STM32F732xx)
#include "stm32f7xx.h"
#else
#error "Please define a MCU arch"
#endif
// Flash magic numbers obtained from family reference manual
#define FLASH_KEY_1 0x45670123
#define FLASH_KEY_2 0xCDEF89AB

void PHAL_flashWriteU32(uint32_t* address, uint32_t value);
void PHAL_flashWriteU64(uint32_t address, uint64_t data);
void PHAL_flashErasePage(uint8_t page);


#endif