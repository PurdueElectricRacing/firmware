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

uint16_t* tx_addr[3];
uint16_t* rx_addr[3];
uint8_t   tx_len[3];
uint8_t   rx_len[3];

usart_debug_t u_debug;

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

    return true;
}

void PHAL_usartTxBl(USART_TypeDef* instance, const uint16_t* data, uint32_t len) {
    uint8_t i;
    
    usart_mask_irq(instance);

    for (i = 0; i < len; i++) {
        while (!(instance->ISR & USART_ISR_TXE));

        instance->TDR = data[i] & 0x01ff;
    }

    while (!(instance->ISR & USART_ISR_TC));
}

void PHAL_usartRxBl(USART_TypeDef* instance, uint16_t* data, uint32_t len) {
    uint8_t i;

    usart_mask_irq(instance);

    for (i = 0; i < len; i++) {
        while (!(instance->ISR & USART_ISR_RXNE));

        data[i] = instance->RDR & 0x01ff;
    }
}

bool PHAL_usartTxInt(USART_TypeDef* instance, uint16_t* data, uint32_t len) {
    uint8_t instance_idx;
    
    usart_unmask_irq(instance);

    switch ((ptr_int) instance)
    {
        case USART1_BASE:
            instance_idx = 0;
            break;

        case USART2_BASE:
            instance_idx = 1;
            break;

        case LPUART1_BASE:
            instance_idx = 2;
            break;

        default:
            return false;
    }

    while (!(instance->ISR & USART_ISR_TXE));
    instance->TDR = data[0] & 0x01ff;

    tx_addr[instance_idx] = data;
    tx_len[instance_idx] = len - 1;

    return true;
}

bool PHAL_usartRxInt(USART_TypeDef* instance, uint16_t* data, uint32_t len) {
    uint8_t instance_idx;
    
    usart_unmask_irq(instance);

    switch ((ptr_int) instance)
    {
        case USART1_BASE:
            instance_idx = 0;
            break;

        case USART2_BASE:
            instance_idx = 1;
            break;

        case LPUART1_BASE:
            instance_idx = 2;
            break;

        default:
            return false;
    }

    rx_addr[instance_idx] = data;
    rx_len[instance_idx] = len;

    return true;
}

bool PHAL_usartTxDma(USART_TypeDef* instance, uint16_t* data, uint32_t len) {
    return false;
}

bool PHAL_usartRxDma(USART_TypeDef* instance, uint16_t* data, uint32_t len) {
    return false;
}

void USART1_IRQHandler(void) {
    usart_isr_handle(USART1, 0);
}

void USART2_IRQHandler(void) {
    usart_isr_handle(USART2, 1);
}

void LPUART1_IRQHandler(void) {
    usart_isr_handle(LPUART1, 2);
}

static void usart_isr_handle(USART_TypeDef* instance, uint8_t instance_idx) {
    static uint8_t bytes_tx[3];
    static uint8_t bytes_rx[3];

    if (instance->ISR & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_PE)) {
        ++u_debug.err_occ;
    }
    
    if (instance->ISR & USART_ISR_TXE) {
        if (tx_len[instance_idx] > 0) {
            ++bytes_tx[instance_idx];
        } else {
            ++u_debug.err_occ;

            return;
        }

        instance->TDR = *(tx_addr[instance_idx] + bytes_tx[instance_idx]) & 0x01ff;

        ++u_debug.bytes_tx;

        if (--tx_len[instance_idx] == 0) {
            bytes_tx[instance_idx] = 0;
        }
    }

    if (instance->ISR & USART_ISR_RXNE) {
        if (rx_len[instance_idx] > 0) {
            ++bytes_rx[instance_idx];
        } else {
            ++u_debug.err_occ;

            return;
        }

        *(rx_addr[instance_idx] + bytes_rx[instance_idx]) = instance->RDR & 0x01ff;

        ++u_debug.bytes_rx;
        
        if (--rx_len[instance_idx] == 0) {
            bytes_rx[instance_idx] = 0;
        }
    }

    instance->ICR &= 0x121bdf;
}

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
