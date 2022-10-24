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

void PHAL_flashWriteU32(uint32_t* address, uint32_t value)
{
    flashUnlock();
    FLASH->CR |= FLASH_CR_PG;
    *(__IO uint32_t*)address = value;
    asm("nop");
    asm("nop");
    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");
    flashLock();
}

void PHAL_flashWriteU64(uint32_t Address, uint64_t Data)
{
    flashUnlock();
    /* Check the parameters */
    // assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));
    if (*((uint32_t*) Address) != 0xFFFFFFFF) asm("bkpt 1");
    if (*((uint32_t*) (Address + 4)) != 0xFFFFFFFF) asm("bkpt 1");

    // ensure not busy
    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    /* Set PG bit */
    FLASH->CR |= FLASH_CR_PG;

    /* Program first word */
    //   *(__IO uint64_t *)Address = Data;
    *(__IO uint32_t*)Address = (uint32_t)Data;
    *(__IO uint32_t*)(Address+4U) = (uint32_t)(Data >> 32);
    asm("nop");
    asm("nop");
    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");
    FLASH->CR &= ~(FLASH_CR_PG);
    flashLock();

    if (*((uint64_t*) Address) != Data || FLASH->CR & 0x1C3FA)
    {
        asm("bkpt 1");
    }
}

void PHAL_flashErasePage(uint8_t page)
{
    if (((page) << FLASH_CR_PNB_Pos) & FLASH_CR_PNB_Msk != (page) << FLASH_CR_PNB_Pos)
    {
        asm("bkpt 1");
        return; // Invalid Page
    }

    flashUnlock();
    FLASH->CR |= FLASH_CR_PER;
    FLASH->CR |= ((page) << FLASH_CR_PNB_Pos) & FLASH_CR_PNB_Msk;
    FLASH->CR |= FLASH_CR_STRT;

    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");
    FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_PNB_Msk);
    flashLock();
    if (FLASH->CR & 0x1C3FA)
    {
        asm("bkpt 1");
    }

}