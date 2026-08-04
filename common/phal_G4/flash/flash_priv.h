/**
 * @file flash_priv.h
 * @brief STM32G4 internal-flash register-level implementation interface.
 * @author Ronak Jain (jain717@purdue.edu)
 *
 * Only flash.c and flash_priv.c should include this header. The public API
 * operates on byte ranges; this interface owns controller lock, cache, bank,
 * page, and status-register details.
 */

#ifndef PHAL_G4_FLASH_PRIV_H
#define PHAL_G4_FLASH_PRIV_H

#include <stdbool.h>
#include <stdint.h>

/** Cache state saved while a flash write or erase is in progress. */
typedef struct {
    bool instruction_cache_enabled; /**< ICEN state before the operation. */
    bool data_cache_enabled; /**< DCEN state before the operation. */
} flash_operation_context_t;

/**
 * @brief Acquire the flash operation lock, wait for idle, and clear status.
 * @return true when a read operation may proceed; false if already in use or the busy wait times out.
 */
bool FLASH_PRIV_begin_read(void);

/**
 * @brief Finish a read operation and release the operation lock.
 * @return true when no flash error flags were set.
 */
bool FLASH_PRIV_end_read(void);

/**
 * @brief Prepare the controller for programming or page erase.
 *
 * This validates the operating voltage, waits for idle, disables data cache,
 * unlocks the controller, and clears stale status flags.
 *
 * @param context Output cache state restored by FLASH_PRIV_end_operation().
 * @return true when the controller is ready; false when unavailable or invalid.
 */
bool FLASH_PRIV_begin_operation(flash_operation_context_t *context);

/**
 * @brief Finalize a completed operation, restore cache state, lock the controller,
 *        and release the operation lock.
 * @param context Cache state returned by FLASH_PRIV_begin_operation().
 * @return true when the controller is idle, error-free, and locked again.
 */
bool FLASH_PRIV_end_operation(const flash_operation_context_t *context);

/**
 * @brief Read the device-reported internal flash capacity.
 * @return Flash size in bytes, using the supported-device fallback if invalid.
 */
uint32_t FLASH_PRIV_get_size_bytes(void);

/**
 * @brief Determine the active page size from the dual-bank option bit.
 * @return 2 KiB for dual-bank mode or 4 KiB for single-bank mode.
 */
uint32_t FLASH_PRIV_get_page_size_bytes(void);

/**
 * @brief Program one STM32G4 double word.
 * @param address 8-byte-aligned flash destination.
 * @param data 64-bit value written low word first.
 * @return true when programming completes without a flash error.
 */
bool FLASH_PRIV_program_double_word(uint32_t address, uint64_t data);

/**
 * @brief Erase one complete flash page.
 *
 * The page number and bank are derived from the absolute page address and the
 * current device bank configuration.
 *
 * @param page_address Address of the page start.
 * @return true when erase completes without a flash error.
 */
bool FLASH_PRIV_erase_page(uint32_t page_address);

#endif // PHAL_G4_FLASH_PRIV_H
