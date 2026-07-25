/**
 * @file flash.h
 * @brief STM32G4 internal-flash public API.
 * @author Ronak Jain (jain717@purdue.edu)
 *
 * The public layer accepts byte ranges and hides controller unlock, cache,
 * bank, page, and status-register details in flash_priv.c.
 */

#ifndef PHAL_G4_FLASH_H
#define PHAL_G4_FLASH_H

#include <stdbool.h>
#include <stdint.h>

/**
 * STM32G4 flash programming granularity.
 *
 * Writes are padded with erased-state bytes (0xFF) to this size when the
 * requested length is not a multiple of the programming granularity.
 */
static constexpr uint32_t PHAL_FLASH_WRITE_ALIGNMENT_BYTES = 8U;

/**
 * @brief Copy bytes from internal flash into a buffer.
 *
 * The requested range must be within internal flash. The address is a CPU
 * address, not an offset from FLASH_BASE. A zero-length read succeeds without
 * accessing either pointer.
 *
 * @param address Address of the first byte to read.
 * @param destination Buffer that receives the data.
 * @param length_bytes Number of bytes to read.
 * @return true when all bytes are read; false for an invalid range, null buffer, or flash error.
 */
bool PHAL_FLASH_read(uint32_t address, void *destination, uint32_t length_bytes);

/**
 * @brief Program bytes into erased internal flash.
 *
 * The destination must be erased before writing. The complete programmed range
 * is checked for erased-state bytes before any programming starts. A
 * zero-length write succeeds without accessing either pointer.
 *
 * @param address Address of the first byte to write; must be 8-byte aligned.
 * @param source Buffer containing the data to write.
 * @param length_bytes Number of bytes to write.
 * @return true when all bytes are programmed and verified; false on invalid input or flash error.
 * @note A partial final double word is padded with 0xFF.
 * @note The source buffer is read only while this call is active.
 */
bool PHAL_FLASH_write(uint32_t address, const void *source, uint32_t length_bytes);

/**
 * @brief Erase every internal flash page touched by a range.
 *
 * The address may be unaligned. Every complete page touched by the range is
 * erased. A zero-length erase succeeds without modifying flash.
 *
 * @param address Address of the first byte in the range.
 * @param length_bytes Number of bytes in the range.
 * @return true when every affected page is erased and verified; false on an invalid range or
 * flash error.
 * @warning Erase operates on complete pages. Bytes outside the range are erased when they share
 * an affected page.
 * @note Erase and write operations are serialized and temporarily manage the
 *       flash controller's cache and lock state.
 */
bool PHAL_FLASH_erase(uint32_t address, uint32_t length_bytes);

#endif // PHAL_G4_FLASH_H
