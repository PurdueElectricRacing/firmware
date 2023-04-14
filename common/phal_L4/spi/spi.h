/**
 * @file spi.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2022-01-22
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef _PHAL_SPI_H
#define _PHAL_SPI_H

#include "stm32l4xx.h"
#include "common/phal_L4/dma/dma.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common_defs.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Configuration entry for SPI initilization
 */
typedef struct
{
    uint32_t data_rate; /* Target baudrate in b/s (might be different depending on peripheral clock divison) */
    uint8_t data_len;   /* Number of bits per transaction */
    bool nss_sw;        /* Save Select controlled by Software */
    GPIO_TypeDef *nss_gpio_port;
    uint32_t nss_gpio_pin;

    dma_init_t *rx_dma_cfg; /* DMA initilization for RX transfer */
    dma_init_t *tx_dma_cfg; /* DMA initilization for TX transfer */

    volatile bool _busy;  /* SPI Peripheral currently in a transaction */
    volatile bool _error; /* SPI Peripheral current transaction error */

    SPI_TypeDef *periph; /*SPI Peripheral*/
} SPI_InitConfig_t;

/**
 * @brief Initilize SPI peripheral with the configuired structure
 *
 * @param handle SPI handle to configure.
 * @return true
 * @return false
 */
bool PHAL_SPI_init(SPI_InitConfig_t *handle);

/**
 * @brief Transfer  data_len bytes from out_data to SPI device and place MISO data in in_data
 * This function just starts the DMA transfer,
 *
 * @param spi SPI handle
 * @param out_data Address of data buffer to put on MOSI line
 * @param data_len Number of SPI packets to send
 * @param in_data Address of data buffer to put data coming in MISO line
 * @return true Able to start DMA transaction
 * @return false Unable to start DMA transaction
 */
bool PHAL_SPI_transfer(SPI_InitConfig_t *spi, const uint8_t *out_data, const uint32_t data_len, const uint8_t *in_data);

/**
 * @brief Check for current SPI transaction to complete
 * @param cfg Spi config
 *
 * @return true
 * @return false
 */
bool PHAL_SPI_busy(SPI_InitConfig_t *cfg);

/**
 * @brief Blocking function to write a single byte to a SPI device.
 * Useful for Initilization functions that just need to write a single byte in a blocking function.
 *
 * @param spi Spi handle
 * @param address Address of data to read
 * @param writeDat Data to write to address
 */
uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat);

/**
 * @brief Blocking function to read a single byte from a SPI device.
 * Useful for Initilization functions that just need to read a single byte in a blocking function.
 *
 * @param spi Spi handle
 * @param address Address of data to read
 * @param skipDummy Return the 3rd byte read instead of the 2nd byte
 */
uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy);

#define SPI1_RXDMA_CONT_CONFIG(rx_addr_, priority_)                               \
    {                                                                             \
        .periph_addr = (uint32_t) & (SPI1->DR), .mem_addr = (uint32_t)(rx_addr_), \
        .tx_size = 1, .increment = false, .circular = false,                      \
        .dir = 0b0, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
        .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
        .tx_isr_en = true, .dma_chan_request = 0b0001, .channel_idx = 2,          \
        .periph = DMA1, .channel = DMA1_Channel2, .request = DMA1_CSELR           \
    }

#define SPI1_TXDMA_CONT_CONFIG(tx_addr_, priority_)                               \
    {                                                                             \
        .periph_addr = (uint32_t) & (SPI1->DR), .mem_addr = (uint32_t)(tx_addr_), \
        .tx_size = 1, .increment = false, .circular = false,                      \
        .dir = 0b1, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
        .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
        .tx_isr_en = false, .dma_chan_request = 0b0001, .channel_idx = 3,         \
        .periph = DMA1, .channel = DMA1_Channel3, .request = DMA1_CSELR           \
    }

#define SPI2_RXDMA_CONT_CONFIG(rx_addr_, priority_)                               \
    {                                                                             \
        .periph_addr = (uint32_t) & (SPI2->DR), .mem_addr = (uint32_t)(rx_addr_), \
        .tx_size = 1, .increment = false, .circular = false,                      \
        .dir = 0b0, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
        .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
        .tx_isr_en = true, .dma_chan_request = 0b0001, .channel_idx = 4,          \
        .periph = DMA1, .channel = DMA1_Channel4, .request = DMA1_CSELR           \
    }

#define SPI2_TXDMA_CONT_CONFIG(tx_addr_, priority_)                               \
    {                                                                             \
        .periph_addr = (uint32_t) & (SPI2->DR), .mem_addr = (uint32_t)(tx_addr_), \
        .tx_size = 1, .increment = false, .circular = false,                      \
        .dir = 0b1, .mem_inc = true, .periph_inc = false, .mem_to_mem = false,    \
        .priority = (priority_), .mem_size = 0b00, .periph_size = 0b00,           \
        .tx_isr_en = false, .dma_chan_request = 0b0001, .channel_idx = 5,         \
        .periph = DMA1, .channel = DMA1_Channel5, .request = DMA1_CSELR           \
    }

#endif /* _PHAL_SPI_H */