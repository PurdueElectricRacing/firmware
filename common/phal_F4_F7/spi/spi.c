/**
 * @file spi.c
 * @author Aditya Anand (anand89@purdue.edu) - Port of L4 HAL by Adam Busch (busch8@purdue.edu)
 * @brief PER Serial Peripheral Interface Device driver for STM32F4 and STM32F7
 * @version 0.1
 * @date 2023-12-4
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "common/phal_F4_F7/spi/spi.h"

extern uint32_t APB2ClockRateHz;
extern uint32_t APB1ClockRateHz;
static volatile SPI_InitConfig_t *active_transfer = NULL;

static uint16_t trash_can; //!< Used as an address for DMA to dump data into
static uint16_t zero;      //!< Used as a constant zero during transmissions

bool PHAL_SPI_init(SPI_InitConfig_t *cfg)
{
    zero = 0;
    // Enable RCC Clock
    if (cfg->periph == SPI1)
    {
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    }
    else
    {
        return false;
    }

    // Setup for Master, positive polarity
    cfg->periph->CR1 |= SPI_CR1_MSTR | SPI_CR1_SPE | SPI_CR1_SSM | SPI_CR1_SSI;
    cfg->periph->CR1 &= ~(SPI_CR1_CPOL);
    cfg->periph->CR2 &= ~(SPI_CR2_SSOE);


    #ifdef STM32F407xx
        if (cfg->data_len != 8 && cfg->data_len != 16)
            return false;
        // Set data frame size for SPI transaction to 8/16 bits depending on user configuration
        cfg->periph->CR1 &= ~(SPI_CR1_DFF);
        cfg->periph->CR1 |= (cfg->data_len != 8) << SPI_CR1_DFF_Pos;
    #endif

    #ifdef STM32F732xx
        // Disable Hardware Controlled NSS
        cfg->periph->CR2 &= ~(SPI_CR2_NSSP);
        // Data Size
        cfg->periph->CR2 &= ~(SPI_CR2_DS_Msk);
        cfg->periph->CR2 |= (CLAMP(cfg->data_len, 4, 16) - 1) << SPI_CR2_DS_Pos;
        // RX Fifo full on 8 bits
        cfg->periph->CR2 |= SPI_CR2_FRXTH;
    #endif

    // Data Rate
    // Divisor is a power of 2, find the closest power of 2 limited to log2(256)
    uint32_t f_div = 0;
    if (cfg->periph == SPI1)
        f_div = LOG2_DOWN(APB2ClockRateHz / cfg->data_rate) - 1;
    // Both SPI2 and SPI3 are on APB1
    else
        f_div = LOG2_DOWN(APB1ClockRateHz / cfg->data_rate) - 1;

    f_div = CLAMP(f_div, 0, 0b111);
    cfg->periph->CR1 &= ~SPI_CR1_BR_Msk;
    cfg->periph->CR1 |= f_div << SPI_CR1_BR_Pos;


    // Setup DMA Streams if required
    if (cfg->rx_dma_cfg && !PHAL_initDMA(cfg->rx_dma_cfg))
        return false;

    if (cfg->tx_dma_cfg && !PHAL_initDMA(cfg->tx_dma_cfg))
        return false;


    // Ensure device CS is disabled
    PHAL_writeGPIO(cfg->nss_gpio_port, cfg->nss_gpio_pin, 1);

    cfg->_busy = false;
    cfg->_error = false;
    cfg->_direct_mode_error = false;
    cfg->_fifo_overrun = false;

    return true;
}

bool PHAL_SPI_transfer_noDMA(SPI_InitConfig_t *spi, const uint8_t *out_data, uint32_t txlen, uint32_t rxlen, uint8_t *in_data)
{
    in_data += txlen;

    if (PHAL_SPI_busy(spi))
        return false;

    active_transfer = spi;
    spi->_busy = true;
    // Enable SPI
    spi->periph->CR1 |= SPI_CR1_SPE;

    // Select peripheral
    if (spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);
    // Add messages to TX FIFO
    for (uint8_t i = 0; i < txlen; i++)
    {
        while (!(spi->periph->SR & SPI_SR_TXE))
            ;
        if (i + 1 < txlen)
        {
            uint16_t data = out_data[i + 1] << 8 | (uint16_t)out_data[i];
            spi->periph->DR = data;
            i++;
        }
        else
        {
            spi->periph->DR = out_data[i];
        }
    }
    // Wait till TX FIFO is empty and spi is not active
    while (!(spi->periph->SR & SPI_SR_TXE) || (spi->periph->SR & SPI_SR_BSY))
        ;
    // Clear overrun
    uint8_t trash = spi->periph->DR;
    trash = spi->periph->SR;

    // RX
    for (uint8_t i = 0; i < rxlen; i++)
    {
        // Wait till SPI is not in active transaction, send dummy
        while ((spi->periph->SR & SPI_SR_BSY))
            ;
        spi->periph->DR = 0;
        // With for RX FIFO to recieve a message, read it in
        while (!(spi->periph->SR & SPI_SR_RXNE))
            ;
        in_data[i] = (uint8_t)(spi->periph->DR);
    }
    // Wait till transaction is finished, disable spi, and clear the queue
    while ((spi->periph->SR & SPI_SR_BSY))
        ;
    spi->periph->CR1 &= ~SPI_CR1_SPE;
    #ifdef STM32F732xx
        while ((spi->periph->SR & SPI_SR_FRLVL))
        {
            trash = spi->periph->DR;
        }
    #endif
    // Stop message by disabling CS
    if (spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 1);

    spi->_busy = false;

    return true;
}

bool PHAL_SPI_transfer(SPI_InitConfig_t *spi, const uint8_t *out_data, const uint32_t data_len, const uint8_t *in_data)
{
    /*
    Each DMA Stream is enabled if a data buffer is provided.
    RX Side interrupts disabled, since the same data lengths are sent to both TX and RX
    */
   // Cannot use DMA without knowing essential configuration info
   if (spi->tx_dma_cfg == 0 || spi->rx_dma_cfg == 0)
   {
    return false;
   }

    if (PHAL_SPI_busy(spi))
        return false;

    active_transfer = spi;

    // Enable CS to begin SPI transaction
    if (spi->nss_sw)
        PHAL_writeGPIO(spi->nss_gpio_port, spi->nss_gpio_pin, 0);

    spi->_busy = true;

    // Configure DMA send msg
    spi->periph->CR2 |= SPI_CR2_TXDMAEN;
    if (!out_data)
    {
        // No data to send, fill with dummy data
        spi->tx_dma_cfg->stream->CR &= ~DMA_SxCR_MINC;
        PHAL_DMA_setMemAddress(spi->tx_dma_cfg, (uint32_t)&zero);
    }
    else
    {
        PHAL_DMA_setMemAddress(spi->tx_dma_cfg, (uint32_t)out_data);
    }
    PHAL_DMA_setTxferLength(spi->tx_dma_cfg, data_len);


    //Configure DMA receive Msg
    spi->periph->CR2 |= SPI_CR2_RXDMAEN;
    if (!in_data)
    {
        // No data to receive, so configure DMA to disregard any received messages
        spi->rx_dma_cfg->stream->CR &= ~DMA_SxCR_MINC;
        PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t)&trash_can);
    }
    else
    {
        PHAL_DMA_setMemAddress(spi->rx_dma_cfg, (uint32_t)in_data);
    }
    PHAL_DMA_setTxferLength(spi->rx_dma_cfg, data_len);

    // We must clear interrupt flags before enabling DMA
    PHAL_reEnable(spi->rx_dma_cfg);

    // // Enable the DMA IRQ
    if (spi->periph == SPI1)
    {
        NVIC_EnableIRQ(DMA2_Stream3_IRQn);
    }

    // Start transaction
    spi->periph->CR1 |= SPI_CR1_SPE;

    // STM32 HAL Libraries start TX Dma transaction last
    PHAL_reEnable(spi->tx_dma_cfg);

    return true;
}

bool PHAL_SPI_busy(SPI_InitConfig_t *cfg)
{
    // Latch in case active_transfer cleared during interrupt
    volatile SPI_InitConfig_t *act = active_transfer;
    if (act && cfg->periph == act->periph)
        return act->_busy;

    return false;
}


//DMA TX ISR
//Streams 0-3 are in the Low interrupt flag register (LIFR)
//Streams 4-7 are in the High interrupt flag register (HIFCR)
void DMA2_Stream3_IRQHandler()
{
    // Transfer Error interrupt
    if (DMA2->LISR & DMA_LISR_TEIF3)
    {
        DMA2->LIFCR |= DMA_LIFCR_CTEIF3;
        if (active_transfer)
            active_transfer->_error = true;
    }
    // Transfer Complete interrupt flag
    if (DMA2->LISR & DMA_LISR_TCIF3)
    {
        // RM0090 p.895, wait for TXE and BSY to be cleared to satisfy timing requirements
        while (!(active_transfer->periph->SR & (SPI_SR_TXE)) || (active_transfer->periph->SR & (SPI_SR_BSY)))
            ;
        // Raise CS to end transaction
        if (active_transfer->nss_sw)
            PHAL_writeGPIO(active_transfer->nss_gpio_port, active_transfer->nss_gpio_pin, 1);

        // Disable DMA channels
        PHAL_stopTxfer(active_transfer->rx_dma_cfg);
        PHAL_stopTxfer(active_transfer->tx_dma_cfg);

        // Revert to mem_inc if trash_can used
        if (active_transfer->rx_dma_cfg->mem_inc)
            active_transfer->rx_dma_cfg->stream->CR |= DMA_SxCR_MINC;
        if (active_transfer->tx_dma_cfg->mem_inc)
            active_transfer->tx_dma_cfg->stream->CR |= DMA_SxCR_MINC;

        // Disable SPI peripheral and DMA requests
        SPI1->CR1 &= ~SPI_CR1_SPE;
        SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        // Clear possible errors, remove current transaction from active transfer
        active_transfer->_busy = false;
        active_transfer->_error = false;
        active_transfer->_direct_mode_error = false;
        active_transfer->_fifo_overrun = false;
        active_transfer = NULL;

        //Clear interrupt flag
        DMA2->LIFCR |= DMA_LIFCR_CTCIF3;
    }
    // Half transfer complete flag
    if (DMA2->LISR & DMA_LISR_HTIF3)
    {
        DMA2->LIFCR |= DMA_LIFCR_CHTIF3;
    }
    // FIFO Overrun Error flag
    if (DMA2->LISR & DMA_LISR_FEIF3)
    {
        if (active_transfer)
            active_transfer->_fifo_overrun = true;
        DMA2->LIFCR |= DMA_LIFCR_CFEIF3;
    }
    // Direct Mode Error flag
    if (DMA2->LISR & DMA_LISR_DMEIF3)
    {
        if (active_transfer)
            active_transfer->_direct_mode_error = true;
        DMA2->LIFCR |= DMA_LIFCR_CDMEIF3;
    }
}

uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy)
{
    static uint8_t tx_cmd[4] = {(1 << 7), 0, 0};
    static uint8_t rx_dat[4] = {1, 1, 1, 1};
    tx_cmd[0] |= (address & 0x7F);

    // Send address, read byte depending on whether DMA is selected
    while (PHAL_SPI_busy(spi))
        ;
    if (spi->rx_dma_cfg != NULL)
        PHAL_SPI_transfer(spi, tx_cmd, skipDummy ? 2 : 3, rx_dat);
    else
        PHAL_SPI_transfer_noDMA(spi, tx_cmd, 1, skipDummy ? 1 : 2, rx_dat);
    while (PHAL_SPI_busy(spi))
        ;

    // Skip first byte of rx in case of unnecesssary information
    return skipDummy ? rx_dat[1] : rx_dat[2];
}

uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat)
{
    uint8_t tx_cmd[3] = {0};
    uint8_t rx_dat[3] = {0};
    tx_cmd[0] |= (address & 0x7F);
    tx_cmd[1] |= (writeDat);

    // Send address, write byte depending on whether DMA is selected
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