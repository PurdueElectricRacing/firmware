/**
 * @file spi.c
 * @author Ronak Jain (jain717@purdue.edu)
 * @author Shriya Balu (balu@purdue.edu)
 * @brief G4 SPI
 * @version 0.1
 */

#include "common/phal_G4/spi/spi.h"
#include "common/phal_G4/spi/spi_priv.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/dma/dma.h"
#include "common/utils/clamp.h"


static uint16_t trash_can; // For RX discard when in_data NULL
static uint16_t zero;      // For TX dummy when out_data NULL

[[gnu::weak]]
void PHAL_SPI_txCallback(SPI_InitConfig_t *spi) {
    (void)spi;
}

/* Map DMA channel IRQs to handler for common SPI usage. Adjust as needed per project. */
[[gnu::weak]]
void DMA1_Channel3_IRQHandler(void) { // example: SPI1_TX on DMA1 Ch3
    PHAL_SPI_priv_handleTxComplete(DMA1, 3);
}

[[gnu::weak]]
void DMA1_Channel5_IRQHandler(void) { // example: SPI2_TX on DMA1 Ch5
    PHAL_SPI_priv_handleTxComplete(DMA1, 5);
}

[[gnu::weak]]
void DMA2_Channel3_IRQHandler(void) { // example: SPI3_TX on DMA2 Ch3
    PHAL_SPI_priv_handleTxComplete(DMA2, 3);
}

void PHAL_SPI_init(SPI_InitConfig_t *cfg) {
    // Enable RCC Clock for selected SPI on G4
    PHAL_SPI_priv_enableClock(cfg->periph);

    /// Peripheral configuration (See RM0440 42.5.7 Configuration of SPI section)
    uint32_t f_div = PHAL_SPI_priv_calcBaudRatePrescaler(cfg->data_rate, cfg->periph);

    // Write to the SPI_CR1 register (baud rate, CPOL/CPHA, simplex/half-duplex, frame format, CRC, slave select, master/slave configs)
    PHAL_SPI_priv_configCR1(cfg, f_div);

    // Write to SPI_CR2 register (transfer data length, slave select output enable, )
    PHAL_SPI_priv_configCR2(cfg);

    // DMA setup is required 
    PHAL_DMA_init(cfg->rx_dma);
    PHAL_DMA_init(cfg->tx_dma);

    // Deassert CS in master when using software NSS
    if (cfg->mode == SPI_MODE_MASTER && cfg->nss_sw)
        PHAL_writeGPIO(cfg->nss_gpio_port, cfg->nss_gpio_pin, 1);

    PHAL_SPI_priv_resetTransferState(cfg);
}


/// Wrapper around SPI transfer that busy waits for completion 
void PHAL_SPI_transfer_blocking(SPI_InitConfig_t *spi,
                             const uint8_t *out_data,
                             uint32_t txlen,
                             uint8_t *in_data) {
    // Start the transfer
    PHAL_SPI_transfer(spi, out_data, txlen, in_data);
    // Wait for this transfer to complete
    while (PHAL_SPI_busy(spi)) {
        __asm__("nop");
    }
}

void PHAL_SPI_transfer(SPI_InitConfig_t *spi,
                       const uint8_t *out_data,
                       const uint32_t data_len,
                       uint8_t *in_data) {

    // Wait for any previous transfer to complete
    while (PHAL_SPI_busy(spi)) {
        __asm__("nop");
    }

    // Assert CS for master only
    if (spi->mode == SPI_MODE_MASTER && spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);

    spi->_busy = true;

    // TX DMA enable
    PHAL_SPI_priv_enableDMA_TX(spi);
    if (!out_data) {
        PHAL_DMA_setMemInc(spi->tx_dma, false);
        PHAL_DMA_setMemAddress(spi->tx_dma, (uint32_t)&zero);
    } else {
        PHAL_DMA_setMemInc(spi->tx_dma, true);
        PHAL_DMA_setMemAddress(spi->tx_dma, (uint32_t)out_data);
    }
    PHAL_DMA_setLength(spi->tx_dma, data_len);

    // RX DMA  
    PHAL_SPI_priv_enableDMA_RX(spi);

    if (!in_data) {
        PHAL_DMA_setMemInc(spi->rx_dma, false);
        PHAL_DMA_setMemAddress(spi->rx_dma, (uint32_t)&trash_can);
    } else {
        PHAL_DMA_setMemInc(spi->rx_dma, true);
        PHAL_DMA_setMemAddress(spi->rx_dma, (uint32_t)in_data);
    }
    PHAL_DMA_setLength(spi->rx_dma, data_len);
    PHAL_DMA_restart(spi->rx_dma);

    PHAL_SPI_priv_registerActiveTx(spi);

    if (PHAL_DMA_getPeriph(spi->tx_dma) == DMA1) {
        NVIC_EnableIRQ(DMA1_Channel1_IRQn + (PHAL_DMA_getChannelIdx(spi->tx_dma) - 1));
    } else if (PHAL_DMA_getPeriph(spi->tx_dma) == DMA2) {
        NVIC_EnableIRQ(DMA2_Channel1_IRQn + (PHAL_DMA_getChannelIdx(spi->tx_dma) - 1));
    } else {
        __builtin_trap();
    }

    // Start SPI and kick TX DMA
    PHAL_SPI_priv_Enable(spi);
    PHAL_DMA_restart(spi->tx_dma);
}

bool PHAL_SPI_busy(SPI_InitConfig_t *cfg) {
    return cfg->_busy;
}
