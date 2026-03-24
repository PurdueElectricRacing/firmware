/**
 * @file flash.c
 * @brief G4 Flash HAL implementation - page-based, double-word programming
 */

#include "flash.h"

#if defined(FLASH_CR_PSIZE_Msk) && defined(FLASH_CR_PSIZE_1) && defined(FLASH_CR_PSIZE_0)
#define FLASH_SET_DOUBLEWORD_PARALLELISM()        \
    do {                                          \
        FLASH->CR &= ~(FLASH_CR_PSIZE_Msk);       \
        FLASH->CR |= FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0; \
    } while (0)
#else
#define FLASH_SET_DOUBLEWORD_PARALLELISM() ((void)0)
#endif

static uint16_t flashGetPage(uint32_t addr) {
    return (addr - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE;
}

static uint8_t flashUnlock() {
    uint32_t timeout = 0;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    FLASH->KEYR = FLASH_KEY_1;
    FLASH->KEYR = FLASH_KEY_2;
    __DSB();
    return FLASH_OK;
}

static void flashLock() {
    __DSB();
    FLASH->CR |= FLASH_CR_LOCK;
}

uint32_t PHAL_flashReadU32(uint32_t addr) {
    return *((__IO uint32_t *)addr);
}

uint8_t PHAL_flashWriteU32(uint32_t address, uint32_t value) {
    uint32_t timeout = 0;
    uint8_t ret;

    // G4 requires 64-bit aligned writes, so we read existing word pair
    uint32_t aligned_addr = address & ~0x7;
    uint32_t is_high_word = (address & 0x4) != 0;

    // Check if the 64-bit aligned region is cleared
    uint64_t existing = *(uint64_t *)aligned_addr;
    if (existing != 0xFFFFFFFFFFFFFFFF) {
        // Need to check if the specific word slot is clear
        if (is_high_word) {
            if ((existing >> 32) != 0xFFFFFFFF)
                return FLASH_ADDR_NOT_CLEARED;
        } else {
            if ((existing & 0xFFFFFFFF) != 0xFFFFFFFF)
                return FLASH_ADDR_NOT_CLEARED;
        }
    }

    ret = flashUnlock();
    if (ret != FLASH_OK)
        return ret;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;
    timeout = 0;

    // Set 64-bit parallelism for G4 when supported by device headers
    FLASH_SET_DOUBLEWORD_PARALLELISM();

    // Enable flash programming
    FLASH->CR |= FLASH_CR_PG;

    // Write double-word
    if (is_high_word) {
        uint64_t new_val          = (existing & 0xFFFFFFFF) | ((uint64_t)value << 32);
        *(uint64_t *)aligned_addr = new_val;
    } else {
        uint64_t new_val          = (existing & 0xFFFFFFFF00000000ULL) | value;
        *(uint64_t *)aligned_addr = new_val;
    }
    __DSB();

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    FLASH->CR &= ~(FLASH_CR_PG);
    flashLock();

    if (*((uint32_t *)address) != value) {
        return FLASH_FAIL;
    }

    return FLASH_OK;
}

uint8_t PHAL_flashWriteU64(uint32_t address, uint64_t data) {
    uint32_t timeout = 0;
    uint8_t ret;

    // Address must be 64-bit aligned for G4
    if (address & 0x7)
        return FLASH_FAIL;

    // Check if address is cleared
    if (*(uint64_t *)address != 0xFFFFFFFFFFFFFFFF)
        return FLASH_ADDR_NOT_CLEARED;

    ret = flashUnlock();
    if (ret != FLASH_OK)
        return ret;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;
    timeout = 0;

    // Set 64-bit parallelism when supported by device headers
    FLASH_SET_DOUBLEWORD_PARALLELISM();

    // Enable flash programming
    FLASH->CR |= FLASH_CR_PG;

    // Write double-word
    *(uint64_t *)address = data;
    __DSB();

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    FLASH->CR &= ~(FLASH_CR_PG);
    flashLock();

    if (*(uint64_t *)address != data) {
        return FLASH_FAIL;
    }

    return FLASH_OK;
}

uint8_t PHAL_flashWriteU32_Buffered(uint32_t address, uint32_t *data, uint32_t count) {
    uint32_t timeout = 0;
    uint8_t ret;

    // G4 requires 64-bit writes, so count must be even
    if (count & 1)
        return FLASH_FAIL;

    ret = flashUnlock();
    if (ret != FLASH_OK)
        return ret;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;
    timeout = 0;

    // Set 64-bit parallelism when supported by device headers
    FLASH_SET_DOUBLEWORD_PARALLELISM();

    // Enable flash programming
    FLASH->CR |= FLASH_CR_PG;

    for (uint32_t i = 0; i < count; i += 2) {
        uint64_t double_word           = ((uint64_t)data[i + 1] << 32) | data[i];
        *(uint64_t *)(address + i * 4) = double_word;
    }
    __DSB();

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT)
        return FLASH_TIMEOUT;

    FLASH->CR &= ~(FLASH_CR_PG);
    flashLock();

    return FLASH_OK;
}

uint8_t PHAL_flashErasePage(uint16_t page) {
    if (page >= FLASH_TOTAL_PAGES)
        return FLASH_FAIL;

    uint32_t timeout = 0;
    uint8_t ret;
    ret = flashUnlock();
    if (ret != FLASH_OK)
        return ret;

    // Enable page erase
    FLASH->CR |= FLASH_CR_PER;

    // Set page number
    FLASH->CR &= ~FLASH_CR_PNB_Msk;
    FLASH->CR |= ((uint32_t)page << FLASH_CR_PNB_Pos) & FLASH_CR_PNB_Msk;

    // Start erase
    FLASH->CR |= FLASH_CR_STRT;

    while ((FLASH->SR & FLASH_SR_BSY) && ++timeout < PHAL_FLASH_TIMEOUT)
        __asm__("nop");
    if (timeout == PHAL_FLASH_TIMEOUT) {
        flashLock();
        return FLASH_TIMEOUT;
    }

    // Clear page erase
    FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_PNB_Msk);

    flashLock();

    return FLASH_OK;
}

uint8_t PHAL_flashErase(uint32_t *addr, uint32_t words) {
    uint8_t ret;
    if (words == 0)
        return FLASH_OK;

    uint32_t *end_addr_inclusive = addr + (words - 1);
    if ((uint32_t)addr < FLASH_BASE_ADDR || (uint32_t)end_addr_inclusive > FLASH_END_ADDR)
        return FLASH_FAIL;

    uint16_t page_start = flashGetPage((uint32_t)addr);
    uint16_t page_end   = flashGetPage((uint32_t)end_addr_inclusive);

    for (uint16_t i = page_start; i <= page_end; ++i) {
        ret = PHAL_flashErasePage(i);
        if (ret != FLASH_OK)
            return ret;
    }
    return FLASH_OK;
}
