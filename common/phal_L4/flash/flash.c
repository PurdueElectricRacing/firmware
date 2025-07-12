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

/**
 * @brief Convert address to sector number
 *        Assumes addr is within flash addr space
 * 
 * @param addr Flash address
 * @return The flash sector
 */
static uint32_t flashGetSector(uint32_t addr) {
    return (addr - USER_FLASH_BASE_ADDRESS) / FLASH_PAGE_SIZE;
}

static uint8_t flashUnlock() {
    uint32_t timeout = 0;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        asm("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    FLASH->KEYR = FLASH_KEY_1; // Keys must be set to unlock flash
    FLASH->KEYR = FLASH_KEY_2;
    __DSB();
    return FLASH_OK;
}

static void flashLock() {
    __DSB();
    FLASH->CR |= FLASH_CR_LOCK;
}

uint8_t PHAL_flashWriteU64(uint32_t Address, uint64_t Data) {
    uint32_t timeout = 0;
    uint8_t ret;
    ret = flashUnlock();
    if (ret != FLASH_OK)
        return ret;

    /* Check the parameters */
    if (*((uint32_t*)Address) != 0xFFFFFFFF)
        return FLASH_ADDR_NOT_CLEARED;
    if (*((uint32_t*)(Address + 4)) != 0xFFFFFFFF)
        return FLASH_ADDR_NOT_CLEARED;

    // Wait until not busy
    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        asm("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;
    timeout = 0;

    // Enable flash programming
    FLASH->CR |= FLASH_CR_PG;

    /* Program first word */
    *(__IO uint32_t*)Address = (uint32_t)Data;

    /* Barrier to ensure programming is performed in 2 steps, in right order
        (independently of compiler optimization behavior) */
    __ISB();

    /* Program second word */
    *(__IO uint32_t*)(Address + 4U) = (uint32_t)(Data >> 32);
    asm("nop");
    asm("nop");

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        asm("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    // Disable flash programming
    FLASH->CR &= ~(FLASH_CR_PG);
    flashLock();

    // Check for proper flash write
    if (*((uint64_t*)Address) != Data || FLASH->CR & 0x1C3FA) {
        return FLASH_FAIL;
    }

    return FLASH_OK;
}

uint8_t PHAL_flashErasePage(uint16_t page) {
    // Validate page request
    if (page > MAX_FLASH_SECTOR) {
        return FLASH_FAIL;
    }

    uint32_t timeout = 0;
    uint8_t ret;
    ret = flashUnlock();
    if (ret != FLASH_OK)
        return ret;

    // Enable page erase
    FLASH->CR |= FLASH_CR_PER;

    // Set bank if defined
#ifdef FLASH_CR_BKER
    FLASH->CR &= ~FLASH_CR_BKER;
    if (page > FLASH_BANK_PG_COUNT) {
        page -= FLASH_BANK_PG_COUNT;
        FLASH->CR |= FLASH_CR_BKER;
    }
#endif
    FLASH->CR &= ~FLASH_CR_PNB_Msk;
    FLASH->CR |= ((page) << FLASH_CR_PNB_Pos) & FLASH_CR_PNB_Msk;

    // Execute the erase
    FLASH->CR |= FLASH_CR_STRT;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        asm("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    // Disable page erase and clear the page request
    FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_PNB_Msk);

    flashLock();

    return FLASH_OK;
}

uint8_t PHAL_flashErase(uint32_t* addr, uint32_t words) {
    uint8_t ret;
    if (words == 0)
        return FLASH_OK;
    uint32_t* end_addr_inclusive = addr + (words - 1);
    if ((uint32_t)addr < FLASH_BASE || (uint32_t)end_addr_inclusive > (FLASH_END - 3))
        return FLASH_FAIL;
    uint8_t sector_start = flashGetSector((uint32_t)addr);
    uint8_t sector_end = flashGetSector((uint32_t)end_addr_inclusive);
    for (uint8_t i = sector_start; i <= sector_end; ++i) {
        ret = PHAL_flashErasePage(i);
        if (ret != FLASH_OK)
            return ret;
    }
    return FLASH_OK;
}
