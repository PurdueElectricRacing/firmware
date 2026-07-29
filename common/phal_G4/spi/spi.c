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
#include "common/utils/clamp.h"
#include "common/common_defs/common_defs.h"


// Track active TX transfers per DMA controller/channel so multiple SPI instances can run concurrently
static volatile SPI_InitConfig_t *dma1_active_tx[8] = {0};
static volatile SPI_InitConfig_t *dma2_active_tx[8] = {0};

static uint16_t trash_can; // For RX discard when in_data NULL
static uint16_t zero;      // For TX dummy when out_data NULL

static inline uint32_t LOG2_DOWN(uint32_t x) {
    return 31U - (uint32_t)__builtin_clz(x);
}

static void handleTxComplete(DMA_TypeDef *dma_periph, uint8_t channel);


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

bool PHAL_SPI_init(SPI_InitConfig_t *cfg) {
    // Enable RCC Clock for selected SPI on G4
    if (!PHAL_SPI_priv_enableClock(cfg->periph)) return false;

    /// Peripheral configuration (See RM0440 42.5.7 Configuration of SPI section)
    uint32_t f_div = PHAL_SPI_priv_calcBaudRatePrescaler(cfg->data_rate, cfg->periph);

    // Write to the SPI_CR1 register (baud rate, CPOL/CPHA, simplex/half-duplex, frame format, CRC, slave select, master/slave configs)
    PHAL_SPI_priv_configCR1(cfg, f_div);

    // Write to SPI_CR2 register (transfer data length, slave select output enable, )
    PHAL_SPI_priv_configCR2(cfg);

    // DMA setup is required 
    if (!PHAL_initDMA(cfg->rx_dma_cfg))
        return false;
    if (!PHAL_initDMA(cfg->tx_dma_cfg))
        return false;

    // Deassert CS in master when using software NSS
    if (cfg->mode == SPI_MODE_MASTER && cfg->nss_sw)
        PHAL_writeGPIO(cfg->nss_gpio_port, cfg->nss_gpio_pin, 1);

    PHAL_SPI_priv_resetTransferState(cfg);

    return true;
}

bool PHAL_SPI_transfer_blocking(SPI_InitConfig_t *spi,
                             const uint8_t *out_data,
                             uint32_t txlen,
                             uint32_t rxlen,
                             uint8_t *in_data) {
    // Prepare RX pointer to skip echoed TX bytes
    uint8_t *rx_ptr = in_data ? (in_data + txlen) : NULL;

    if (PHAL_SPI_busy(spi))
        return false;

    spi->_busy = true;

    // Assert CS for master only
    if (spi->mode == SPI_MODE_MASTER && spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);

    // Enable SPI
    spi->periph->CR1 |= SPI_CR1_SPE;

    // Transmit txlen bytes and capture echoed RX into in_data if provided
    for (uint32_t i = 0; i < txlen; i++) {
        // Wait for TXE
        while (!(spi->periph->SR & SPI_SR_TXE))
            ;
        // Write byte
        uint8_t b = out_data ? out_data[i] : 0;
        *(volatile uint8_t *)&spi->periph->DR = b;
        // Wait for RXNE and read echo
        while (!(spi->periph->SR & SPI_SR_RXNE)) {
            __asm__("nop");
        }
        uint8_t rxb = *(volatile uint8_t *)&spi->periph->DR;
        if (in_data)
            in_data[i] = rxb;
    }

    // If additional rxlen bytes requested beyond txlen, clock out dummy
    for (uint32_t i = 0; i < rxlen; i++) {
        while (!(spi->periph->SR & SPI_SR_TXE)) {
            __asm__("nop");
        }
        *(volatile uint8_t *)&spi->periph->DR = 0;
        while (!(spi->periph->SR & SPI_SR_RXNE)) {
            __asm__("nop");
        }
        uint8_t rb = *(volatile uint8_t *)&spi->periph->DR;
        if (rx_ptr)
            rx_ptr[i] = rb;
    }

    
    return true;
}

bool PHAL_SPI_transfer(SPI_InitConfig_t *spi,
                       const uint8_t *out_data,
                       const uint32_t data_len,
                       uint8_t *in_data) {
    if (spi->tx_dma == 0)
        return false;
    if (PHAL_SPI_busy(spi))
        return false;

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
    if (!spi->rx_dma_cfg) return false;
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

    // Enable DMA IRQ for selected channel and track active transfer per-channel
    volatile SPI_InitConfig_t **active_table;
    if (PHAL_DMA_getPeriph(spi->tx_dma) == DMA1) {
        NVIC_EnableIRQ(DMA1_Channel1_IRQn + (PHAL_DMA_getChannelIdx(spi->tx_dma) - 1));
        active_table = dma1_active_tx;
    } else if (PHAL_DMA_getPeriph(spi->tx_dma) == DMA2) {
        NVIC_EnableIRQ(DMA2_Channel1_IRQn + (PHAL_DMA_getChannelIdx(spi->tx_dma) - 1));
        active_table = dma2_active_tx;
    } else {
        return false;
    }
    active_table[PHAL_DMA_getChannelIdx(spi->tx_dma)] = spi;

    // Start SPI and kick TX DMA
    PHAL_SPI_priv_Enable(spi);
    PHAL_DMA_restart(spi->tx_dma);

    return true;
}

bool PHAL_SPI_busy(SPI_InitConfig_t *cfg) {
    return cfg->_busy;
}

void PHAL_SPI_ForceReset(SPI_InitConfig_t *spi) {
    switch ((uint32_t)spi->periph) {
        case SPI1_BASE:
            RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
            break;
        case SPI2_BASE:
            RCC->APB1RSTR1 |= RCC_APB1RSTR1_SPI2RST;
            RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_SPI2RST;
            break;
        case SPI3_BASE:
            RCC->APB1RSTR1 |= RCC_APB1RSTR1_SPI3RST;
            RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_SPI3RST;
            break;
        default:
            break;
    }
}

static void handleTxComplete(DMA_TypeDef *dma_periph, uint8_t channel) {
    volatile SPI_InitConfig_t **active_table =
        (dma_periph == DMA1) ? dma1_active_tx : dma2_active_tx;
    SPI_InitConfig_t *transfer = (channel < 8) ? (SPI_InitConfig_t *)active_table[channel] : NULL;
    if (transfer == NULL)
        return;

    uint32_t tcif_mask = DMA_ISR_TCIF1 << (4 * (channel - 1));
    uint32_t teif_mask = DMA_ISR_TEIF1 << (4 * (channel - 1));
    uint32_t htif_mask = DMA_ISR_HTIF1 << (4 * (channel - 1));
    uint32_t gif_mask  = DMA_ISR_GIF1 << (4 * (channel - 1));

    if (dma_periph->ISR & teif_mask) {
        dma_periph->IFCR |= teif_mask;
        transfer->_error = true;
    }
    if (dma_periph->ISR & tcif_mask) {
        // Wait for TXE and not busy
        while (!(transfer->periph->SR & SPI_SR_TXE) || (transfer->periph->SR & SPI_SR_BSY))
            ;

        // If RX DMA is used, wait until its TC flag also asserts before teardown
        if (transfer->rx_dma) {
            DMA_TypeDef *rx_dma = PHAL_DMA_getPeriph(transfer->rx_dma);
            uint8_t rx_ch       = PHAL_DMA_getChannelIdx(transfer->rx_dma);
            uint32_t rx_tc_mask = DMA_ISR_TCIF1 << (4 * (rx_ch - 1));
            // Busy-wait for RX complete
            while (!(rx_dma->ISR & rx_tc_mask))
                ;
            // Clear RX flags and stop RX
            rx_dma->IFCR |= rx_tc_mask;
            PHAL_DMA_stop(transfer->rx_dma);
        }

        // Deassert CS after both TX and RX complete
        if (transfer->nss_sw)
            PHAL_writeGPIO(transfer->nss_gpio_port, transfer->nss_gpio_pin, 1);

        if (transfer->tx_dma)
            PHAL_DMA_stop(transfer->tx_dma);

        transfer->periph->CR1 &= ~SPI_CR1_SPE;
        transfer->periph->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        transfer->_busy              = false;
        transfer->_error             = false;
        transfer->_direct_mode_error = false;
        transfer->_fifo_overrun      = false;

        dma_periph->IFCR |= tcif_mask;
        dma_periph->IFCR |= gif_mask;
        active_table[channel] = NULL;

        PHAL_SPI_txCallback(transfer);
    }

    if (dma_periph->ISR & htif_mask) {
        dma_periph->IFCR |= htif_mask;
    }
}



uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy) {
    static uint8_t tx_cmd[4] = {(1 << 7), 0, 0};
    static uint8_t rx_dat[4] = {1, 1, 1, 1};
    tx_cmd[0] |= (address & 0x7F);

    while (PHAL_SPI_busy(spi)) {
        __asm__("nop");
    }

    if (spi->rx_dma != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, skipDummy ? 2 : 3, rx_dat);
    while (PHAL_SPI_busy(spi)) {
        __asm__("nop");
    }

    return skipDummy ? rx_dat[1] : rx_dat[2];
}

uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat) {
    uint8_t tx_cmd[3] = {0};
    uint8_t rx_dat[3] = {0};
    tx_cmd[0] |= (address & 0x7F);
    tx_cmd[1] |= (writeDat);

    while (PHAL_SPI_busy(spi)) {
        __asm__("nop");
    }
    if (spi->tx_dma != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, 2, rx_dat);
    while (PHAL_SPI_busy(spi)) {
        __asm__("nop");
    }

    return rx_dat[1];
}
