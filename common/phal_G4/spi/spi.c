/**
 * @file spi.c
 * @author Ronak Jain
 * @brief G4 SPI
 * @version 0.1
 */

#include "common/phal_G4/spi/spi.h"

extern uint32_t APB2ClockRateHz;
extern uint32_t APB1ClockRateHz;

// Track active TX transfers per DMA controller/channel so multiple SPI instances can run concurrently
static volatile SPI_InitConfig_t *dma1_active_tx[8] = {0};
static volatile SPI_InitConfig_t *dma2_active_tx[8] = {0};

static uint16_t trash_can; // For RX discard when in_data NULL
static uint16_t zero;      // For TX dummy when out_data NULL

static void handleTxComplete(DMA_TypeDef *dma_periph, uint8_t channel);

bool PHAL_SPI_init(SPI_InitConfig_t *cfg) {
    zero = 0;

    // Enable RCC Clock for selected SPI on G4
    switch ((uint32_t)cfg->periph) {
        case SPI1_BASE:
            RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
            break;
        case SPI2_BASE:
            RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;
            break;
        case SPI3_BASE:
            RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;
            break;
        default:
            return false;
    }

    // Mode configuration
    if (cfg->mode == SPI_MODE_MASTER) {
        // Master mode, software NSS
        cfg->periph->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
        // Baud rate prescaler (BR) in CR1, source depends on bus
        uint32_t f_div;
        if ((uint32_t)cfg->periph == SPI1_BASE)
            f_div = LOG2_DOWN(APB2ClockRateHz / cfg->data_rate) - 1;
        else
            f_div = LOG2_DOWN(APB1ClockRateHz / cfg->data_rate) - 1;
        f_div = CLAMP(f_div, 0, 0b111);
        cfg->periph->CR1 &= ~SPI_CR1_BR_Msk;
        cfg->periph->CR1 |= f_div << SPI_CR1_BR_Pos;
    } else {
        // Slave mode: clear MSTR.
        cfg->periph->CR1 &= ~SPI_CR1_MSTR;
        if (cfg->nss_sw) {
            // Software NSS: internally select the slave (SSM=1, SSI=0)
            cfg->periph->CR1 |= SPI_CR1_SSM;
            cfg->periph->CR1 &= ~SPI_CR1_SSI;
        } else {
            // Hardware NSS: NSS managed by external pin (SSM=0). SSI ignored.
            cfg->periph->CR1 &= ~SPI_CR1_SSM;
        }
        // BR ignored in slave
    }
    cfg->periph->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
    if (cfg->cpol)
        cfg->periph->CR1 |= SPI_CR1_CPOL;
    if (cfg->cpha)
        cfg->periph->CR1 |= SPI_CR1_CPHA;

    // Frame size via CR2 DS[3:0] on G4
    cfg->periph->CR2 &= ~(SPI_CR2_DS_Msk);
    uint8_t ds = (CLAMP(cfg->data_len, 4, 16) - 1) & 0xF; // DS = bits-1
    cfg->periph->CR2 |= (ds << SPI_CR2_DS_Pos);
    // RX FIFO threshold: set FRXTH for 8-bit threshold
    cfg->periph->CR2 |= SPI_CR2_FRXTH;

    // DMA setup if provided
    if (cfg->rx_dma_cfg && !PHAL_initDMA(cfg->rx_dma_cfg))
        return false;
    if (cfg->tx_dma_cfg && !PHAL_initDMA(cfg->tx_dma_cfg))
        return false;

    // Deassert CS in master when using software NSS
    if (cfg->mode == SPI_MODE_MASTER && cfg->nss_sw)
        PHAL_writeGPIO(cfg->nss_gpio_port, cfg->nss_gpio_pin, 1);

    cfg->_busy              = false;
    cfg->_error             = false;
    cfg->_direct_mode_error = false;
    cfg->_fifo_overrun      = false;

    return true;
}

bool PHAL_SPI_transfer_noDMA(SPI_InitConfig_t *spi,
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
        uint8_t b                             = out_data ? out_data[i] : 0;
        *(volatile uint8_t *)&spi->periph->DR = b;
        // Wait for RXNE and read echo
        while (!(spi->periph->SR & SPI_SR_RXNE))
            ;
        uint8_t rxb = *(volatile uint8_t *)&spi->periph->DR;
        if (in_data)
            in_data[i] = rxb;
    }

    // If additional rxlen bytes requested beyond txlen, clock out dummy
    for (uint32_t i = 0; i < rxlen; i++) {
        while (!(spi->periph->SR & SPI_SR_TXE))
            ;
        *(volatile uint8_t *)&spi->periph->DR = 0;
        while (!(spi->periph->SR & SPI_SR_RXNE))
            ;
        uint8_t rb = *(volatile uint8_t *)&spi->periph->DR;
        if (rx_ptr)
            rx_ptr[i] = rb;
    }

    // Wait until not busy and TXE set
    while ((spi->periph->SR & SPI_SR_BSY))
        ;
    while (!(spi->periph->SR & SPI_SR_TXE))
        ;

    // Deassert CS for master only
    if (spi->mode == SPI_MODE_MASTER && spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 1);

    // Disable SPI
    spi->periph->CR1 &= ~SPI_CR1_SPE;

    spi->_busy = false;
    return true;
}

bool PHAL_SPI_transfer(SPI_InitConfig_t *spi,
                       const uint8_t *out_data,
                       const uint32_t data_len,
                       uint8_t *in_data) {
    if (spi->tx_dma_cfg == 0)
        return false;
    if (PHAL_SPI_busy(spi))
        return false;

    // Assert CS for master only
    if (spi->mode == SPI_MODE_MASTER && spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);

    spi->_busy = true;

    // TX DMA enable
    spi->periph->CR2 |= SPI_CR2_TXDMAEN;
    if (!out_data) {
        spi->tx_dma_cfg->mem_inc = false;
        PHAL_DMA_setMemAddress(spi->tx_dma_cfg, (uint32_t)&zero);
    } else {
        spi->tx_dma_cfg->mem_inc = true;
        PHAL_DMA_setMemAddress(spi->tx_dma_cfg, (uint32_t)out_data);
    }
    PHAL_DMA_setTxferLength(spi->tx_dma_cfg, data_len);

    // RX DMA optional
    if (spi->rx_dma_cfg) {
        spi->periph->CR2 |= SPI_CR2_RXDMAEN;
        if (!in_data) {
            spi->rx_dma_cfg->mem_inc = false;
            PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t)&trash_can);
        } else {
            spi->rx_dma_cfg->mem_inc = true;
            PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t)in_data);
        }
        PHAL_DMA_setTxferLength(spi->rx_dma_cfg, data_len);
        PHAL_reEnable(spi->rx_dma_cfg);
    }

    // Enable DMA IRQ for selected channel and track active transfer per-channel
    volatile SPI_InitConfig_t **active_table;
    if (spi->tx_dma_cfg->periph == DMA1) {
        NVIC_EnableIRQ(DMA1_Channel1_IRQn + (spi->tx_dma_cfg->channel_idx - 1));
        active_table = dma1_active_tx;
    } else if (spi->tx_dma_cfg->periph == DMA2) {
        NVIC_EnableIRQ(DMA2_Channel1_IRQn + (spi->tx_dma_cfg->channel_idx - 1));
        active_table = dma2_active_tx;
    } else {
        return false;
    }
    active_table[spi->tx_dma_cfg->channel_idx] = spi;

    // Start SPI and kick TX DMA
    spi->periph->CR1 |= SPI_CR1_SPE;
    PHAL_reEnable(spi->tx_dma_cfg);

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
        if (transfer->rx_dma_cfg) {
            DMA_TypeDef *rx_dma = transfer->rx_dma_cfg->periph;
            uint8_t rx_ch       = transfer->rx_dma_cfg->channel_idx;
            uint32_t rx_tc_mask = DMA_ISR_TCIF1 << (4 * (rx_ch - 1));
            // Busy-wait for RX complete
            while (!(rx_dma->ISR & rx_tc_mask))
                ;
            // Clear RX flags and stop RX
            rx_dma->IFCR |= rx_tc_mask;
            PHAL_stopTxfer(transfer->rx_dma_cfg);
        }

        // Deassert CS after both TX and RX complete
        if (transfer->nss_sw)
            PHAL_writeGPIO(transfer->nss_gpio_port, transfer->nss_gpio_pin, 1);

        if (transfer->tx_dma_cfg)
            PHAL_stopTxfer(transfer->tx_dma_cfg);

        transfer->periph->CR1 &= ~SPI_CR1_SPE;
        transfer->periph->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        transfer->_busy              = false;
        transfer->_error             = false;
        transfer->_direct_mode_error = false;
        transfer->_fifo_overrun      = false;

        dma_periph->IFCR |= tcif_mask;
        dma_periph->IFCR |= gif_mask;
        active_table[channel] = NULL;
    }

    if (dma_periph->ISR & htif_mask) {
        dma_periph->IFCR |= htif_mask;
    }
}

/* Map DMA channel IRQs to handler for common SPI usage. Adjust as needed per project. */
__attribute__((weak)) void DMA1_Channel3_IRQHandler(void) { // example: SPI1_TX on DMA1 Ch3
    handleTxComplete(DMA1, 3);
}

__attribute__((weak)) void DMA1_Channel5_IRQHandler(void) { // example: SPI2_TX on DMA1 Ch5
    handleTxComplete(DMA1, 5);
}

__attribute__((weak)) void DMA2_Channel3_IRQHandler(void) { // example: SPI3_TX on DMA2 Ch3
    handleTxComplete(DMA2, 3);
}

uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy) {
    static uint8_t tx_cmd[4] = {(1 << 7), 0, 0};
    static uint8_t rx_dat[4] = {1, 1, 1, 1};
    tx_cmd[0] |= (address & 0x7F);

    while (PHAL_SPI_busy(spi))
        ;
    if (spi->rx_dma_cfg != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, skipDummy ? 2 : 3, rx_dat);
    else
        PHAL_SPI_transfer_noDMA(spi, tx_cmd, 1, skipDummy ? 1 : 2, rx_dat);
    while (PHAL_SPI_busy(spi))
        ;

    return skipDummy ? rx_dat[1] : rx_dat[2];
}

uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat) {
    uint8_t tx_cmd[3] = {0};
    uint8_t rx_dat[3] = {0};
    tx_cmd[0] |= (address & 0x7F);
    tx_cmd[1] |= (writeDat);

    while (PHAL_SPI_busy(spi))
        ;
    if (spi->tx_dma_cfg != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, 2, rx_dat);
    else
        PHAL_SPI_transfer_noDMA(spi, tx_cmd, 2, 0, rx_dat);
    while (PHAL_SPI_busy(spi))
        ;

    return rx_dat[1];
}
