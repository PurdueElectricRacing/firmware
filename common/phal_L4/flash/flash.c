/**
 * @file hal_flash.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "common/phal_L4/flash/flash.h"

static void flashUnlock()
{
    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    FLASH->KEYR = FLASH_KEY_1;
    FLASH->KEYR = FLASH_KEY_2;
    __DSB();
    
}

static void flashLock()
{
    __DSB();
    FLASH->CR |= FLASH_CR_LOCK;
}

void flashWriteU32(uint32_t address, uint32_t value)
{
    flashUnlock();
    // Set program size to 32bit
    // FLASH->CR &= ~(FLASH_CR_PSIZE_Msk);
    // FLASH->CR |= FLASH_CR_PSIZE_1;

    FLASH->CR |= FLASH_CR_PG;
    *(__IO uint32_t*)address = value;
    asm("nop");
    asm("nop");
    flashLock();
}