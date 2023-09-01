/**
 * @file flash.c
 * @author Chris McGalliard - port of L4 by Adam Busch (busch8@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2023-08-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "common/phal_F4_F7/flash/flash.h"

static void flashUnlock()
{
    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    FLASH->KEYR = FLASH_KEY_1;                  // Keys must be set to unlock flash
    FLASH->KEYR = FLASH_KEY_2;
    __DSB();                                    // Data synchonization barrier (wait for any memory instructions to finish)
}

static void flashLock()
{
    __DSB();
    FLASH->CR |= FLASH_CR_LOCK;                 // Set-only bit, will be cleared upon next succcessful flash unlock
}

void PHAL_flashWriteU32(uint32_t* address, uint32_t value)
{
    flashUnlock();

    // Set 32 bit parallelism (see RM 0090 Page 85)
    FLASH->CR &= ~(FLASH_CR_PSIZE_Msk);
    FLASH->CR |= ((FLASH_CR_PSIZE_1) << FLASH_CR_PSIZE_Pos) & FLASH_CR_PSIZE_Msk;

    // Enable flash programming
    FLASH->CR |= FLASH_CR_PG;

    // Write word
    *(__IO uint32_t*)address = value;
    asm("nop");
    asm("nop");

    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    // Disable flash programming
    FLASH->CR &= ~(FLASH_CR_PG);

    // Check for proper flash write
    if (*((uint32_t*) address) != value)
    {
        asm("bkpt 1");
    }

    flashLock();
}

void PHAL_flashWriteU64(uint32_t Address, uint64_t Data)
{
    flashUnlock();

    // Set 32 bit parallelism (see RM 0090 Page 85)
    FLASH->CR &= ~(FLASH_CR_PSIZE_Msk);
    FLASH->CR |= ((FLASH_CR_PSIZE_1) << FLASH_CR_PSIZE_Pos) & FLASH_CR_PSIZE_Msk;

    // Ensure the requested 64 bits are ready
    if (*((uint32_t*) Address) != 0xFFFFFFFF) asm("bkpt 1");
    if (*((uint32_t*) (Address + 4)) != 0xFFFFFFFF) asm("bkpt 1");

    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    // Enable flash programming
    FLASH->CR |= FLASH_CR_PG;

    // Program First word
    *(__IO uint32_t*)Address = (uint32_t)Data;

    // Program Second word
    *(__IO uint32_t*)(Address+4U) = (uint32_t)(Data >> 32);
    asm("nop");
    asm("nop");

    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    // Disable flash programming
    FLASH->CR &= ~(FLASH_CR_PG);
    flashLock();

    // Check for proper flash write
    if (*((uint64_t*) Address) != Data)
    {
        asm("bkpt 1");
    }
}

void PHAL_flashErasePage(uint8_t page)
{

    // Validate page request
    if (page > 0b1011)
    {
        asm("bkpt 1");
    }

    flashUnlock();

    // Set 32 bit parallelism (see RM 0090 Page 85)
    FLASH->CR &= ~(FLASH_CR_PSIZE_Msk);
    FLASH->CR |= ((FLASH_CR_PSIZE_1) << FLASH_CR_PSIZE_Pos) & FLASH_CR_PSIZE_Msk;

    // Enable sector erase
    FLASH->CR |= FLASH_CR_SER;

    // Set the sector to be erased (0-11)
    FLASH->CR |= ((page) << FLASH_CR_SNB_Pos) & FLASH_CR_SNB_Msk;

    // Execute the erase
    FLASH->CR |= FLASH_CR_STRT;

    while ((FLASH->SR & FLASH_SR_BSY))
        asm("nop");

    // Disable page erase and clear the page request
    FLASH->CR &= ~(FLASH_CR_SER | FLASH_CR_SNB_Msk);

    flashLock();
}