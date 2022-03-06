/**
 * @file usart.c
 * @author Dawson Moore (moore800@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-12-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "common/phal_L4/usart/usart.h"

static void usart_isr_handle(USART_TypeDef* instance, uint8_t instance_idx);
static void usart_unmask_irq(USART_TypeDef* instance);
static void usart_mask_irq(USART_TypeDef* instance);

bool PHAL_initUSART(USART_TypeDef* instance, usart_init_t* handle, const uint32_t fck)
{
     // Locals
    uint16_t div;

    // Disable peripheral until properly configured
    instance->CR1 &= ~USART_CR1_UE;

    // Enable peripheral clock in RCC
    switch ((ptr_int) instance) {
        case USART1_BASE:
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            break;

        case USART2_BASE:
            RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
            break;

        case LPUART1_BASE:
            RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
            break;

        default:
            return false;
    }

    // Calculate prescalar for given baud rate
    div = (fck / handle->baud_rate) * ((handle->ovsample == OV_16) ? 1 : 2);

    // Apply prescalar
    switch (handle->ovsample) {
        case OV_16:
            instance->BRR = div;
            break;

        case OV_8:
            instance->BRR = ((div & 0b111) >> 1) | (div & 0xfff0);
            break;
    }

    // Set CR1 parameters
    instance->CR1 =  0U;
    instance->CR1 |= (handle->word_length & 2) << USART_CR1_M1_Pos;
    instance->CR1 |= (handle->word_length & 1) << USART_CR1_M0_Pos;
    instance->CR1 |= handle->ovsample << USART_CR1_OVER8_Pos;
    instance->CR1 |= handle->parity << USART_CR1_PS_Pos;
    instance->CR1 |= handle->mode << USART_CR1_RE_Pos;
    instance->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE;

    // Set CR2 parameters
    instance->CR2 =  0U;
    instance->CR2 |= handle->adv_feature.ab_mode << USART_CR2_ABRMODE_Pos;
    instance->CR2 |= handle->adv_feature.auto_baud << USART_CR2_ABREN_Pos;
    instance->CR2 |= handle->adv_feature.msb_first << USART_CR2_MSBFIRST_Pos;
    instance->CR2 |= handle->adv_feature.data_inv << USART_CR2_DATAINV_Pos;
    instance->CR2 |= handle->adv_feature.tx_inv << USART_CR2_TXINV_Pos;
    instance->CR2 |= handle->adv_feature.rx_inv << USART_CR2_RXINV_Pos;
    instance->CR2 |= handle->adv_feature.tx_rx_swp << USART_CR2_SWAP_Pos;
    instance->CR2 |= handle->stop_bits << USART_CR2_STOP_Pos;

    // Set CR3 parameters
    instance->CR3 =  0U;
    instance->CR3 |= handle->adv_feature.dma_on_rx_err << USART_CR3_DDRE_Pos;
    instance->CR3 |= handle->adv_feature.overrun << USART_CR3_OVRDIS_Pos;
    instance->CR3 |= handle->obsample << USART_CR3_ONEBIT_Pos;
    instance->CR2 |= USART_CR3_EIE;

    // Enable peripheral for use
    instance->CR1 |= USART_CR1_UE;

    // Configure DMA
    if (handle->tx_dma_cfg && !PHAL_initDMA(handle->tx_dma_cfg)) {
        return false;
    }

    if (handle->rx_dma_cfg && !PHAL_initDMA(handle->rx_dma_cfg)) {
        return false;
    }

    // Ensure interrupts are masked
    usart_mask_irq(instance);

    return true;
}

void PHAL_usartTxBl(USART_TypeDef* instance, const uint16_t* data, uint32_t len) {
    uint8_t i;
    
    instance->CR3 &= ~USART_CR3_DMAT;

    for (i = 0; i < len; i++) {
        while (!(instance->ISR & USART_ISR_TXE));

        instance->TDR = data[i] & 0x01ff;
    }

    while (!(instance->ISR & USART_ISR_TC));
}

void PHAL_usartRxBl(USART_TypeDef* instance, uint16_t* data, uint32_t len) {
    uint8_t i;

    instance->CR3 &= ~USART_CR3_DMAR;

    for (i = 0; i < len; i++) {
        while (!(instance->ISR & USART_ISR_RXNE));

        data[i] = instance->RDR & 0x01ff;
    }
}

bool PHAL_usartTxDma(USART_TypeDef* instance, usart_init_t* handle, uint16_t* data, uint32_t len) {
    PHAL_stopTxfer(handle->tx_dma_cfg);
    
    instance->CR3 |= USART_CR3_DMAT;
    PHAL_DMA_setTxferLength(handle->tx_dma_cfg, len);
    PHAL_DMA_setMemAddress(handle->tx_dma_cfg, (uint32_t) data);

    instance->ICR |= USART_ICR_TCCF;
    PHAL_startTxfer(handle->tx_dma_cfg);

    return true;
}

bool PHAL_usartTxDmaComplete(usart_init_t* handle)
{
    if (handle->tx_dma_cfg->periph->ISR & (DMA_ISR_TCIF1 << 4 * (handle->tx_dma_cfg->channel_idx - 1)))
    {
        handle->tx_dma_cfg->periph->IFCR = (DMA_IFCR_CTCIF1 << 4 * (handle->tx_dma_cfg->channel_idx - 1));
        return true;
    }
    return false;
}

bool PHAL_usartRxDma(USART_TypeDef* instance, usart_init_t* handle, uint16_t* data, uint32_t len) {
    PHAL_stopTxfer(handle->rx_dma_cfg);

    instance->CR3 |= USART_CR3_DMAR;
    PHAL_DMA_setTxferLength(handle->rx_dma_cfg, len);
    PHAL_DMA_setMemAddress(handle->rx_dma_cfg, (uint32_t) data);
    //NVIC_EnableIRQ(irq);
    PHAL_startTxfer(handle->rx_dma_cfg);

    return true;
}

bool PHAL_usartRxDmaComplete(usart_init_t* handle)
{
    if (handle->rx_dma_cfg->periph->ISR & (DMA_ISR_TCIF1 << 4 * (handle->rx_dma_cfg->channel_idx - 1)))
    {
        handle->rx_dma_cfg->periph->IFCR = (DMA_IFCR_CTCIF1 << 4 * (handle->rx_dma_cfg->channel_idx - 1));
        return true;
    }
    return false;
}

void DMA1_Channel5_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF5)
    {
        DMA1->IFCR |= DMA_IFCR_CTEIF5;
    }
    if (DMA1->ISR & DMA_ISR_TCIF5) 
    {
        DMA1->IFCR |= DMA_IFCR_CTCIF5;
    }
    if (DMA1->ISR & DMA_ISR_GIF5)
    {
        DMA1->IFCR |= DMA_IFCR_CGIF5;
    }
}

// Masking and unmasking ISRs. Useful for interrupt based functionality.
// Likely don't need it since we use DMA, but could come in handy sometime
static void usart_unmask_irq(USART_TypeDef* instance) {
    switch ((ptr_int) instance)
    {
        case USART1_BASE:
            NVIC->ISER[1] |= 1U << (USART1_IRQn - 32);
            break;

        case USART2_BASE:
            NVIC->ISER[1] |= 1U << (USART2_IRQn - 32);
            break;

        case LPUART1_BASE:
            NVIC->ISER[2] |= 1U << (LPUART1_IRQn - 64);
            break;
    }
}

static void usart_mask_irq(USART_TypeDef* instance) {
    switch ((ptr_int) instance)
    {
        case USART1_BASE:
            NVIC->ISER[1] &= ~(1U << (USART1_IRQn - 32));
            break;

        case USART2_BASE:
            NVIC->ISER[1] &= ~(1U << (USART2_IRQn - 32));
            break;

        case LPUART1_BASE:
            NVIC->ISER[2] &= ~(1U << (LPUART1_IRQn - 64));
            break;
    }
}
