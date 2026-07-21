#include "common/phal_G4/flash/flash_priv.h"

#include "common/phal_G4/phal_G4.h"

static constexpr uint32_t PHAL_FLASH_KEY_1 = 0x45670123U;
static constexpr uint32_t PHAL_FLASH_KEY_2 = 0xCDEF89ABU;

static constexpr uint32_t PHAL_FLASH_DEFAULT_SIZE_BYTES = 512U * 1024U;
static constexpr uint32_t PHAL_FLASH_MAX_SIZE_BYTES     = 512U * 1024U;
static constexpr uint32_t PHAL_FLASH_DUAL_PAGE_BYTES    = 2U * 1024U;
static constexpr uint32_t PHAL_FLASH_SINGLE_PAGE_BYTES  = 4U * 1024U;
static constexpr uint32_t PHAL_FLASH_MAX_WAIT_ITERATIONS = 170'000'000U;

static constexpr uint32_t PHAL_FLASH_STATUS_ERROR_MASK = FLASH_SR_OPERR | FLASH_SR_PROGERR
    | FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR | FLASH_SR_MISERR
    | FLASH_SR_FASTERR | FLASH_SR_RDERR | FLASH_SR_OPTVERR;

static constexpr uint32_t PHAL_FLASH_CONTROL_OPERATION_MASK = FLASH_CR_PG | FLASH_CR_PER
    | FLASH_CR_MER1 | FLASH_CR_PNB_Msk | FLASH_CR_BKER | FLASH_CR_MER2 | FLASH_CR_FSTPG;

static volatile bool g_flash_operation_active = false;

static bool flash_acquire(void) {
    uint32_t previous_interrupt_mask = __get_PRIMASK();
    __disable_irq();

    bool acquired = !g_flash_operation_active;
    if (acquired) {
        g_flash_operation_active = true;
    }

    __set_PRIMASK(previous_interrupt_mask);
    return acquired;
}

static void flash_release(void) {
    uint32_t previous_interrupt_mask = __get_PRIMASK();
    __disable_irq();
    g_flash_operation_active = false;
    __set_PRIMASK(previous_interrupt_mask);
}

static bool flash_is_busy(void) {
    return (FLASH->SR & FLASH_SR_BSY) != 0U;
}

static bool flash_wait_until_not_busy(void) {
    uint32_t iterations_remaining = SystemCoreClock;
    if (iterations_remaining == 0U || iterations_remaining > PHAL_FLASH_MAX_WAIT_ITERATIONS) {
        iterations_remaining = PHAL_FLASH_MAX_WAIT_ITERATIONS;
    }

    while (flash_is_busy() && iterations_remaining > 0U) {
        iterations_remaining--;
    }

    return !flash_is_busy();
}

static void flash_clear_status_flags(void) {
    uint32_t clear_flags = FLASH->SR & (PHAL_FLASH_STATUS_ERROR_MASK | FLASH_SR_EOP);
    if (clear_flags != 0U) {
        FLASH->SR = clear_flags;
    }
}

static bool flash_wait_for_last_operation(void) {
    if (!flash_wait_until_not_busy()) {
        return false;
    }

    uint32_t status = FLASH->SR;
    uint32_t errors = status & PHAL_FLASH_STATUS_ERROR_MASK;
    uint32_t clear_flags = errors | (status & FLASH_SR_EOP);
    if (clear_flags != 0U) {
        FLASH->SR = clear_flags;
    }

    return errors == 0U;
}

static bool flash_unlock(void) {
    if ((FLASH->CR & FLASH_CR_LOCK) == 0U) {
        return true;
    }

    FLASH->KEYR = PHAL_FLASH_KEY_1;
    FLASH->KEYR = PHAL_FLASH_KEY_2;
    return (FLASH->CR & FLASH_CR_LOCK) == 0U;
}

static bool flash_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
    return (FLASH->CR & FLASH_CR_LOCK) != 0U;
}

static void flash_disable_data_cache(flash_operation_context_t *context) {
    context->instruction_cache_enabled = (FLASH->ACR & FLASH_ACR_ICEN) != 0U;
    context->data_cache_enabled        = (FLASH->ACR & FLASH_ACR_DCEN) != 0U;

    if (context->data_cache_enabled) {
        FLASH->ACR &= ~FLASH_ACR_DCEN;
    }

    __DSB();
    __ISB();
}

static void flash_restore_caches(const flash_operation_context_t *context) {
    if (context->instruction_cache_enabled) {
        FLASH->ACR &= ~FLASH_ACR_ICEN;
        FLASH->ACR |= FLASH_ACR_ICRST;
        FLASH->ACR &= ~FLASH_ACR_ICRST;
        FLASH->ACR |= FLASH_ACR_ICEN;
    }

    if (context->data_cache_enabled) {
        FLASH->ACR |= FLASH_ACR_DCRST;
        FLASH->ACR &= ~FLASH_ACR_DCRST;
        FLASH->ACR |= FLASH_ACR_DCEN;
    }

    __DSB();
    __ISB();
}

static bool flash_dual_bank_enabled(void) {
    return (FLASH->OPTR & FLASH_OPTR_DBANK) != 0U;
}

static bool flash_voltage_allows_programming(void) {
    uint32_t previous_interrupt_mask = __get_PRIMASK();
    __disable_irq();

    bool power_clock_enabled = (RCC->APB1ENR1 & RCC_APB1ENR1_PWREN) != 0U;
    if (!power_clock_enabled) {
        RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
        __DSB();
    }

    bool range_one = (PWR->CR1 & PWR_CR1_VOS_Msk) == PWR_CR1_VOS_0;
    bool range_stable = (PWR->SR2 & PWR_SR2_VOSF) == 0U;

    if (!power_clock_enabled) {
        RCC->APB1ENR1 &= ~RCC_APB1ENR1_PWREN;
        __DSB();
    }
    __set_PRIMASK(previous_interrupt_mask);
    return range_one && range_stable;
}

static bool flash_banks_swapped(void) {
    uint32_t previous_interrupt_mask = __get_PRIMASK();
    __disable_irq();

    bool system_config_clock_enabled = (RCC->APB2ENR & RCC_APB2ENR_SYSCFGEN) != 0U;
    if (!system_config_clock_enabled) {
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
        __DSB();
    }

    bool banks_swapped = (SYSCFG->MEMRMP & SYSCFG_MEMRMP_FB_MODE) != 0U;

    if (!system_config_clock_enabled) {
        RCC->APB2ENR &= ~RCC_APB2ENR_SYSCFGEN;
        __DSB();
    }
    __set_PRIMASK(previous_interrupt_mask);
    return banks_swapped;
}

bool FLASH_PRIV_begin_read(void) {
    if (!flash_acquire()) {
        return false;
    }

    if (!flash_wait_until_not_busy()) {
        flash_release();
        return false;
    }

    flash_clear_status_flags();
    return true;
}

bool FLASH_PRIV_end_read(void) {
    bool success = (FLASH->SR & PHAL_FLASH_STATUS_ERROR_MASK) == 0U;
    flash_clear_status_flags();
    flash_release();
    return success;
}

bool FLASH_PRIV_begin_operation(flash_operation_context_t *context) {
    if (context == nullptr || !flash_acquire()) {
        return false;
    }

    *context = (flash_operation_context_t) {0};
    if (!flash_voltage_allows_programming() || !flash_wait_until_not_busy()) {
        flash_release();
        return false;
    }

    flash_disable_data_cache(context);
    if (!flash_unlock()) {
        flash_restore_caches(context);
        flash_release();
        return false;
    }

    FLASH->CR &= ~PHAL_FLASH_CONTROL_OPERATION_MASK;
    flash_clear_status_flags();
    return true;
}

bool FLASH_PRIV_end_operation(const flash_operation_context_t *context) {
    if (context == nullptr) {
        return false;
    }

    bool controller_idle = !flash_is_busy();
    bool success = controller_idle && (FLASH->SR & PHAL_FLASH_STATUS_ERROR_MASK) == 0U;
    bool locked = false;
    if (controller_idle) {
        FLASH->CR &= ~PHAL_FLASH_CONTROL_OPERATION_MASK;
        flash_clear_status_flags();
        locked = flash_lock();
    }
    flash_restore_caches(context);
    flash_release();
    return success && locked;
}

uint32_t FLASH_PRIV_get_size_bytes(void) {
    uint32_t size_kib   = *(volatile const uint16_t *)FLASHSIZE_BASE;
    uint32_t size_bytes = size_kib * 1024U;

    if (size_kib == UINT16_MAX || size_bytes == 0U || size_bytes > PHAL_FLASH_MAX_SIZE_BYTES) {
        return PHAL_FLASH_DEFAULT_SIZE_BYTES;
    }

    return size_bytes;
}

uint32_t FLASH_PRIV_get_page_size_bytes(void) {
    return flash_dual_bank_enabled() ? PHAL_FLASH_DUAL_PAGE_BYTES : PHAL_FLASH_SINGLE_PAGE_BYTES;
}

bool FLASH_PRIV_program_double_word(uint32_t address, uint64_t data) {
    flash_clear_status_flags();
    FLASH->CR |= FLASH_CR_PG;

    volatile uint32_t *destination = (volatile uint32_t *)address;
    destination[0]                 = (uint32_t)data;
    __ISB();
    destination[1] = (uint32_t)(data >> 32U);

    bool success = flash_wait_for_last_operation();
    if (!flash_is_busy()) {
        FLASH->CR &= ~FLASH_CR_PG;
    }
    return success;
}

bool FLASH_PRIV_erase_page(uint32_t page_address) {
    uint32_t flash_size_bytes = FLASH_PRIV_get_size_bytes();
    uint32_t page_size_bytes  = FLASH_PRIV_get_page_size_bytes();
    uint32_t flash_offset     = page_address - FLASH_BASE;
    uint32_t page             = flash_offset / page_size_bytes;
    bool bank_two             = false;

    if (flash_dual_bank_enabled()) {
        uint32_t bank_size_bytes = flash_size_bytes / 2U;
        bank_two                 = flash_offset >= bank_size_bytes;
        bank_two                 = bank_two != flash_banks_swapped();
        page                     = (flash_offset % bank_size_bytes) / page_size_bytes;
    }

    flash_clear_status_flags();
    FLASH->CR &= ~(FLASH_CR_PNB_Msk | FLASH_CR_BKER);
    FLASH->CR |= FLASH_CR_PER | ((page << FLASH_CR_PNB_Pos) & FLASH_CR_PNB_Msk);
    if (bank_two) {
        FLASH->CR |= FLASH_CR_BKER;
    }
    FLASH->CR |= FLASH_CR_STRT;

    bool success = flash_wait_for_last_operation();
    if (!flash_is_busy()) {
        FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_PNB_Msk | FLASH_CR_BKER);
    }
    return success;
}
