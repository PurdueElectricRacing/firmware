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

tx_mode_t mode;
bool      comp;

bool PHAL_initUSART(const usart_handle_t* handle, const uint32_t fck)
{
    // Locals
    uint16_t div;

    // Disable peripheral until properly configured
    handle->instance->CR1 &= ~USART_CR1_UE;

    // Enable peripheral clock in RCC
    switch ((ptr_int) handle->instance) {
        case USART1_BASE:
            RCC->AHB2ENR |= RCC_APB2ENR_USART1EN;
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
            handle->instance->BRR = div;
            break;

        case OV_8:
            handle->instance->BRR = ((div & 0b111) >> 1) | (div & 0xfff0);
            break;
    }

    // Set CR1 parameters
    handle->instance->CR1 =  0U;
    handle->instance->CR1 |= (handle->word_length & 2) << USART_CR1_M1_Pos;
    handle->instance->CR1 |= (handle->word_length & 1) << USART_CR1_M0_Pos;
    handle->instance->CR1 |= handle->ovsample << USART_CR1_OVER8_Pos;
    handle->instance->CR1 |= handle->parity << USART_CR1_PS_Pos;
    handle->instance->CR1 |= handle->mode << USART_CR1_RE_Pos;

    // Set CR2 parameters
    handle->instance->CR2 =  0U;
    handle->instance->CR2 |= handle->adv_feature.ab_mode << USART_CR2_ABRMODE_Pos;
    handle->instance->CR2 |= handle->adv_feature.auto_baud << USART_CR2_ABREN_Pos;
    handle->instance->CR2 |= handle->adv_feature.msb_first << USART_CR2_MSBFIRST_Pos;
    handle->instance->CR2 |= handle->adv_feature.data_inv << USART_CR2_DATAINV_Pos;
    handle->instance->CR2 |= handle->adv_feature.tx_inv << USART_CR2_TXINV_Pos;
    handle->instance->CR2 |= handle->adv_feature.rx_inv << USART_CR2_RXINV_Pos;
    handle->instance->CR2 |= handle->adv_feature.tx_rx_swp << USART_CR2_SWAP_Pos;
    handle->instance->CR2 |= handle->stop_bits << USART_CR2_STOP_Pos;

    // Set CR3 parameters
    handle->instance->CR3 =  0U;
    handle->instance->CR3 |= handle->adv_feature.dma_on_rx_err << USART_CR3_DDRE_Pos;
    handle->instance->CR3 |= handle->adv_feature.overrun << USART_CR3_OVRDIS_Pos;
    handle->instance->CR3 |= handle->obsample << USART_CR3_ONEBIT_Pos;

    // Enable peripheral for use
    handle->instance->CR1 |= USART_CR1_UE;
}

void PHAL_usartTxBl(const usart_handle_t* handle, const uint16_t* data, uint32_t len) {
    uint8_t i;
    mode = BLOCKING;
    comp = false;
    
    for (i = 0; i < len; i++) {
        while (!(handle->instance->ISR & USART_ISR_TXE));

        handle->instance->TDR = data[i] & 0x01ff;
    }

    while (!(handle->instance->ISR & USART_ISR_TC));
}

void PHAL_usartRxBl(const usart_handle_t* handle, uint16_t* data, uint32_t len) {
    uint8_t i;
    mode = BLOCKING;
    comp = false;

    for (i = 0; i < len; i++) {
        while (!(handle->instance->ISR & USART_ISR_RXNE));

        data[i] = handle->instance->TDR & 0x01ff;
    }
}

bool PHAL_usartTxInt(const usart_handle_t* handle, const uint16_t* data, uint32_t len) {
    return false;
}

bool PHAL_usartRxint(const usart_handle_t* handle, uint16_t* data, uint32_t len) {
    return false;
}

bool PHAL_usartTxDMA(const usart_handle_t* handle, const uint16_t* data, uint32_t len) {
    return false;
}

bool PHAL_usartRxDMA(const usart_handle_t* handle, uint16_t* data, uint32_t len) {
    return false;
}
