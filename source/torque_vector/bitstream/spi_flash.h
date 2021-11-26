
#include <inttypes.h>
#include <stdbool.h>
#include "common/phal_L4/quadspi/quadspi.h"
#include "common/phal_L4/gpio/gpio.h"

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

/**
 * @brief Check that the SPI Flash status register has a set of flags set
 * 
 * @param status_register_mask  Which bits of the status register are being checked
 * @param status_register_flags Desired values of the chekced status register bits
 * @return true Flag matches
 * @return false Flag does not match
 */
bool spiFlashCheckSR(uint8_t status_register_mask, uint8_t status_register_flags);

/**
 * @brief Enable writing/erasing to spi flash
 * 
 * @return uint32_t 
 */
void spiFlashWriteEnable();

/**
 * @brief Erase a 64k large block from SPI flash
 * 
 * @param block_n 
 * @return uint32_t 
 */
uint32_t spiFlashEraseBlock64k(uint32_t block_n);

#endif