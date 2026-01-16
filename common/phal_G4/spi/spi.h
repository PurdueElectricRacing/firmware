/**
 * @file spi.c
 * @author Ronak Jain
 * @brief G4 SPI
 * @version 0.1
 */

#ifndef _PHAL_G4_SPI_H
#define _PHAL_G4_SPI_H

#include <stddef.h>

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/phal_G4.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common_defs.h"

typedef uint32_t ptr_int;

typedef enum {
    SPI_MODE_MASTER = 0,
    SPI_MODE_SLAVE  = 1,
} SPI_Mode;

typedef struct {
    uint32_t data_rate;
    uint8_t data_len;
    SPI_Mode mode;
    bool nss_sw;
    GPIO_TypeDef *nss_gpio_port;
    uint32_t nss_gpio_pin;

    uint8_t cpol;
    uint8_t cpha;

    dma_init_t *rx_dma_cfg; // DMA RX config (optional)
    dma_init_t *tx_dma_cfg; // DMA TX config (required for DMA path)

    volatile bool _busy;  // Busy flag
    volatile bool _error; // TX error occurred
    volatile bool _direct_mode_error;
    volatile bool _fifo_overrun;

    SPI_TypeDef *periph; // SPI peripheral base
} SPI_InitConfig_t;

bool PHAL_SPI_init(SPI_InitConfig_t *handle);
bool PHAL_SPI_transfer(SPI_InitConfig_t *spi,
                       const uint8_t *out_data,
                       const uint32_t data_len,
                       uint8_t *in_data);
bool PHAL_SPI_transfer_noDMA(SPI_InitConfig_t *spi,
                             const uint8_t *out_data,
                             uint32_t txlen,
                             uint32_t rxlen,
                             uint8_t *in_data);
bool PHAL_SPI_busy(SPI_InitConfig_t *cfg);
uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat);
uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy);
void PHAL_SPI_ForceReset(SPI_InitConfig_t *spi);

#define SPI1_RXDMA_CONT_CONFIG(rx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI1->DR), \
     .mem_addr         = (uint32_t)(rx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 0, /* P2M */ \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0, \
     .channel_idx      = 2, /* DMA1 Channel2 typical for SPI1_RX */ \
     .mux_request      = DMA_REQUEST_SPI1_RX, \
     .periph           = DMA1}

#define SPI1_TXDMA_CONT_CONFIG(tx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI1->DR), \
     .mem_addr         = (uint32_t)(tx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 1, /* M2P */ \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = true, \
     .dma_chan_request = 0, \
     .channel_idx      = 3, /* DMA1 Channel3 typical for SPI1_TX */ \
     .mux_request      = DMA_REQUEST_SPI1_TX, \
     .periph           = DMA1}

/* SPI2 */
#define SPI2_RXDMA_CONT_CONFIG(rx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI2->DR), \
     .mem_addr         = (uint32_t)(rx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 0, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0, \
     .channel_idx      = 4, /* DMA1 Channel4 typical for SPI2_RX */ \
     .mux_request      = DMA_REQUEST_SPI2_RX, \
     .periph           = DMA1}

#define SPI2_TXDMA_CONT_CONFIG(tx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI2->DR), \
     .mem_addr         = (uint32_t)(tx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 1, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = true, \
     .dma_chan_request = 0, \
     .channel_idx      = 5, /* DMA1 Channel5 typical for SPI2_TX */ \
     .mux_request      = DMA_REQUEST_SPI2_TX, \
     .periph           = DMA1}

/* SPI3 */
#define SPI3_RXDMA_CONT_CONFIG(rx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI3->DR), \
     .mem_addr         = (uint32_t)(rx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 0, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = false, \
     .dma_chan_request = 0, \
     .channel_idx      = 2, /* DMA2 Channel2 example */ \
     .mux_request      = DMA_REQUEST_SPI3_RX, \
     .periph           = DMA2}

#define SPI3_TXDMA_CONT_CONFIG(tx_addr_, priority_) \
    {.periph_addr      = (uint32_t)&(SPI3->DR), \
     .mem_addr         = (uint32_t)(tx_addr_), \
     .tx_size          = 1, \
     .mem_size         = DMA_SIZE_8BIT, \
     .increment        = false, \
     .circular         = false, \
     .dir              = 1, \
     .mem_inc          = true, \
     .periph_inc       = false, \
     .mem_to_mem       = false, \
     .priority         = (priority_), \
     .periph_size      = DMA_SIZE_8BIT, \
     .tx_isr_en        = true, \
     .dma_chan_request = 0, \
     .channel_idx      = 3, /* DMA2 Channel3 example */ \
     .mux_request      = DMA_REQUEST_SPI3_TX, \
     .periph           = DMA2}

#endif /* _PHAL_G4_SPI_H */
