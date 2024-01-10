/**
 * @file usart.c
 * @author Aditya Anand (anand89@purdue.edu) - Port of L4 UART HAL by Dawson Moore (moore800@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2024-1-4
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "common/phal_F4_F7/usart/usart.h"
#include "common/common_defs/common_defs.h"

static void usart_isr_handle(USART_TypeDef* instance, uint8_t instance_idx);
static void usart_unmask_irq(USART_TypeDef* instance);
static void usart_mask_irq(USART_TypeDef* instance);

/*static*/ volatile usart_active_transfer_t active_uarts[TOTAL_NUM_UART];

bool PHAL_initUSART(usart_init_t* handle, const uint32_t fck)
{
     // Locals
    uint8_t over8;
    uint32_t div;
    uint32_t carry;
    uint32_t div_fraction;
    uint32_t div_mantissa;

    // Add Handle to active peripheral set for keeping track of activity. The same handle must be passed throughout the use of the USART peripheral
    // If a different configuration of peripheral is to be used/a different handle, peripheral must be reinitalized. 
    active_uarts[handle->usart_active_num].active_handle = handle;

    // Disable peripheral until properly configured
    handle->periph->CR1 &= ~USART_CR1_UE;

    // Enable peripheral clock in RCC
    switch ((ptr_int) handle->periph) {
        case USART1_BASE:
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            break;
        case USART2_BASE:
            RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
            break;
        default:
            return false;
    }

    // The following FBRG configuration is based off RM0090 p. 978 - 980
    over8  = 8 << !handle->ovsample;
    carry = 0;
    // Calculate USARTDIV constant for given baud rate, convert to fixed point for easy manipulation
    div = (uint32_t)(((float)fck / (handle->baud_rate * over8)) * 100);

    //Calculate DIV_Fraction value, round to nearest real number, then acquire carry value if div_fraction > 4bits
    div_fraction = (over8 * (div % 100));
    div_fraction = (( div_fraction > 50) ? ROUNDUP(div_fraction, 100) : ROUNDDOWN(div_fraction, 100)) / 100; 
    if (div_fraction > 15 && handle->ovsample == OV_16)
    {
        carry = div_fraction - 15;
        div_fraction = 15;
    }
    // The last DIV_Fraction bit is ignored in OV8 mode
    else if (div_fraction > 7 && handle->ovsample == OV_8)
    {
        carry = div_fraction - 7;
        div_fraction = 7;
    }
    //Calculate DIV_Mantissa value, accounting for any carry values from the previous DIV_Fraction calculation
    div_mantissa = (div / 100) + carry;
    // Apply prescalar
    handle->periph->BRR = 0U; //Clear any previously programmed values
    handle->periph->BRR |= div_fraction << USART_BRR_DIV_Fraction_Pos;
    handle->periph->BRR |= div_mantissa << USART_BRR_DIV_Mantissa_Pos;

    // Set CR1 parameters
    handle->periph->CR1 =  0U;
    handle->periph->CR1 |= (handle->parity != PT_NONE) << USART_CR1_PCE_Pos;
    handle->periph->CR1 |= handle->word_length << USART_CR1_M_Pos;
    handle->periph->CR1 |= (handle->parity >> 2) << USART_CR1_PS_Pos;
    handle->periph->CR1 |= handle->ovsample << USART_CR1_OVER8_Pos;
    handle->periph->CR1 |= handle->wake_addr << USART_CR1_WAKE_Pos;
    // handle->periph->CR1 |= handle->mode << USART_CR1_RE_Pos;
    handle->periph->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE | USART_CR1_IDLEIE | USART_CR1_PEIE/* | USART_CR1_TE*/;

    // Set CR2 parameters
    handle->periph->CR2 =  0U;
    handle->periph->CR2 |= (handle->address & 0xF) << USART_CR2_ADD_Pos;
    handle->periph->CR2 |= handle->stop_bits << USART_CR2_STOP_Pos;   

    // // Set CR3 parameters
    handle->periph->CR3 |= handle->obsample << USART_CR3_ONEBIT_Pos;
    handle->periph->CR3 |= USART_CR3_EIE;

    // Enable peripheral for use
    handle->periph->CR1 |= USART_CR1_UE;

    // Configure DMA
    if (handle->tx_dma_cfg && !PHAL_initDMA(handle->tx_dma_cfg)) {
        return false;
    }

    if (handle->rx_dma_cfg && !PHAL_initDMA(handle->rx_dma_cfg)) {
        return false;
    }

    return true;
}

void PHAL_usartTxBl(usart_init_t* handle, const uint16_t* data, uint32_t len) {
    uint8_t i;

    handle->periph->CR3 &= ~USART_CR3_DMAT;

    // for (i = 0; i < len; i++) {
    //     while (!(handle->periph->ISR & USART_ISR_TXE));

    //     handle->periph->TDR = data[i] & 0x01ff;
    // }

    // while (!(handle->periph->ISR & USART_ISR_TC));
}

void PHAL_usartRxBl(usart_init_t* handle, uint16_t* data, uint32_t len) {
    uint8_t i;

    handle->periph->CR3 &= ~USART_CR3_DMAR;

    for (i = 0; i < len; i++) {
        // while (!(handle->periph->ISR & USART_ISR_RXNE));

        // data[i] = handle->periph->RDR & 0x01ff;
    }
}

bool PHAL_usartTxDma(usart_init_t* handle, uint16_t* data, uint32_t len) {
    if (active_uarts[handle->usart_active_num].active_handle != handle)
        return false;
    handle->periph->CR1 |= USART_CR1_TE;
    // Enable All Interrupts needed to complete Tx transaction  
    switch ((ptr_int) handle->periph) {
        case USART2_BASE:
            NVIC_EnableIRQ(DMA1_Stream6_IRQn);
            NVIC_EnableIRQ(USART2_IRQn);
            break;
        default:
            return false;
    }
    PHAL_DMA_setTxferLength(handle->tx_dma_cfg, len);
    PHAL_DMA_setMemAddress(handle->tx_dma_cfg, (uint32_t) data);

    // Start DMA transaction
    PHAL_startTxfer(handle->tx_dma_cfg);
    // if (active_uarts[active_idx].active_handle != 0 && active_uarts[active_idx].active_handle != active_uarts)
    active_uarts[handle->usart_active_num]._tx_busy = 1;
    // Configure DMA Peripheral, and set up USART for DMA Transactions
    handle->periph->CR3 |= USART_CR3_DMAT;
    return true;
}

bool PHAL_usartTxBusy(usart_init_t* handle)
{
    if (active_uarts[handle->usart_active_num].active_handle != 0)
        return active_uarts[handle->usart_active_num]._tx_busy;
    return false;
}

bool PHAL_usartRxDma(usart_init_t* handle, uint16_t* data, uint32_t len) {
    // Provided handle must match with the one we have, and we cannot be continously receiving when calling this function
    if (active_uarts[handle->usart_active_num].active_handle != handle || active_uarts[handle->usart_active_num].cont_rx)
        return false;
    // PHAL_stopTxfer(handle->rx_dma_cfg);
    handle->periph->CR1 |= USART_CR1_RE;
    // Enable All Interrupts needed to complete Tx transaction  
    switch ((ptr_int) handle->periph) {
        case USART2_BASE:
            // NVIC_EnableIRQ(DMA1_Stream5_IRQn);
            NVIC_EnableIRQ(USART2_IRQn);
            break;
        default:
            return false;
    }
    handle->periph->CR3 |= USART_CR3_DMAR;
    PHAL_DMA_setTxferLength(handle->rx_dma_cfg, len);
    PHAL_DMA_setMemAddress(handle->rx_dma_cfg, (uint32_t) data);
    //NVIC_EnableIRQ(irq);
    PHAL_reEnable(handle->rx_dma_cfg);
    active_uarts[handle->usart_active_num]._rx_busy = 1;
    return true;
}

bool PHAL_usartRxDmaComplete(usart_init_t* handle)
{
    if (active_uarts[handle->usart_active_num].active_handle != 0)
        return active_uarts[handle->usart_active_num]._rx_busy;
    return false;
}

/**
 * @brief Handle DMA interrupts
 *
 */
static void handleDMAxComplete(uint8_t idx, uint32_t irq, uint8_t dma_type)
{
    // Bitmask for each DMA interrupt flag
    uint32_t teif_flag;
    uint32_t tcif_flag;
    uint32_t htif_flag;
    uint32_t feif_flag;
    uint32_t dmeif_flag;

    // Clear register for DMA Stream
    volatile uint32_t *sr_reg;
    volatile uint32_t *csr_reg;
    uint8_t dma_num;

    // Populate values based on whether DMA Rx or Tx is being handled (Different configurations, streams)
    if (dma_type == USART_DMA_TX)
    {
        dma_num = active_uarts[idx].active_handle->tx_dma_cfg->stream_idx;

        // Select appropriate flag and clear registers for Stream
        sr_reg = (dma_num < 3) ? &active_uarts[idx].active_handle->tx_dma_cfg->periph->LISR : &active_uarts[idx].active_handle->tx_dma_cfg->periph->HISR;
        csr_reg = (dma_num < 3) ? &active_uarts[idx].active_handle->tx_dma_cfg->periph->LIFCR : &active_uarts[idx].active_handle->tx_dma_cfg->periph->HIFCR;
    }
    else // DMA RX is ongoing
    {
        dma_num = active_uarts[idx].active_handle->rx_dma_cfg->stream_idx;

        // Select appropriate flag and clear registers for Stream
        sr_reg = (dma_num < 3) ? &active_uarts[idx].active_handle->rx_dma_cfg->periph->LISR : &active_uarts[idx].active_handle->rx_dma_cfg->periph->HISR;
        csr_reg = (dma_num < 3) ? &active_uarts[idx].active_handle->rx_dma_cfg->periph->LIFCR : &active_uarts[idx].active_handle->rx_dma_cfg->periph->HIFCR;
    }
    // Populate Flag Bitmasks, along with Flag and Clear registers for active DMA Stream
    switch(dma_num)
    {
        case 0:
            // Populate Flag Bitmasks
            teif_flag = DMA_LISR_TEIF0;
            tcif_flag = DMA_LISR_TCIF0;
            htif_flag = DMA_LISR_HTIF0;
            feif_flag = DMA_LISR_FEIF0;
            dmeif_flag = DMA_LISR_DMEIF0;
            break;
        case 1:
            // Populate Flag Bitmasks
            teif_flag = DMA_LISR_TEIF1;
            tcif_flag = DMA_LISR_TCIF1;
            htif_flag = DMA_LISR_HTIF1;
            feif_flag = DMA_LISR_FEIF1;
            dmeif_flag = DMA_LISR_DMEIF1;
            break;
        case 2:
            // Populate Flag Bitmasks
            teif_flag = DMA_LISR_TEIF2;
            tcif_flag = DMA_LISR_TCIF2;
            htif_flag = DMA_LISR_HTIF2;
            feif_flag = DMA_LISR_FEIF2;
            dmeif_flag = DMA_LISR_DMEIF2;
            break;
        case 3:
            // Populate Flag Bitmasks
            teif_flag = DMA_LISR_TEIF3;
            tcif_flag = DMA_LISR_TCIF3;
            htif_flag = DMA_LISR_HTIF3;
            feif_flag = DMA_LISR_FEIF3;
            dmeif_flag = DMA_LISR_DMEIF3;
            break;
        case 4:
            // Populate Flag Bitmasks
            teif_flag = DMA_HISR_TEIF4;
            tcif_flag = DMA_HISR_TCIF4;
            htif_flag = DMA_HISR_HTIF4;
            feif_flag = DMA_HISR_FEIF4;
            dmeif_flag = DMA_HISR_DMEIF4;
            break;
        case 5:
            // Populate Flag Bitmasks
            teif_flag = DMA_HISR_TEIF5;
            tcif_flag = DMA_HISR_TCIF5;
            htif_flag = DMA_HISR_HTIF5;
            feif_flag = DMA_HISR_FEIF5;
            dmeif_flag = DMA_HISR_DMEIF5;
            break;
        case 6:
            // Populate Flag Bitmasks
            teif_flag = DMA_HISR_TEIF6;
            tcif_flag = DMA_HISR_TCIF6;
            htif_flag = DMA_HISR_HTIF6;
            feif_flag = DMA_HISR_FEIF6;
            dmeif_flag = DMA_HISR_DMEIF6;
            break;
        case 7:
            // Populate Flag Bitmasks
            teif_flag = DMA_HISR_TEIF7;
            tcif_flag = DMA_HISR_TCIF7;
            htif_flag = DMA_HISR_HTIF7;
            feif_flag = DMA_HISR_FEIF7;
            dmeif_flag = DMA_HISR_DMEIF4;
            break;
        default:
            // Invalid stream selected, do not attempt to service interrupt
            return;
    }

    bool clear_act_transfer = false;
    // Transfer Error interrupt
    if (*sr_reg & teif_flag)
    {
        *csr_reg |= teif_flag;
    }
    // Transfer Complete interrupt flag
    if (*sr_reg & tcif_flag)
    {
        if (dma_type == USART_DMA_TX)
            PHAL_stopTxfer(active_uarts[idx].active_handle->tx_dma_cfg);
        else if (active_uarts[idx].cont_rx == 0)
            PHAL_stopTxfer(active_uarts[idx].active_handle->rx_dma_cfg);
        // We no longer need the interrupt for this DMA stream, so disable it for now
        NVIC_DisableIRQ(irq);
        //Clear interrupt flag
        *csr_reg |= tcif_flag;
    }
    // Half transfer complete flag
    if ((*sr_reg) & htif_flag)
    {
        *csr_reg |= htif_flag;
    }
    // FIFO Overrun Error flag
    if (*sr_reg & feif_flag)
    {
        *csr_reg |= feif_flag;
    }
    // Direct Mode Error flag
    if (*sr_reg & dmeif_flag)
    {
        *csr_reg |= dmeif_flag;
    }
}

static void handleUsartIRQ(USART_TypeDef *handle, uint8_t idx)
{
    #ifdef STM32F407xx
    if (handle->SR & USART_SR_CTS)
    {
        handle->SR &= ~USART_SR_CTS; // We currently do not plan on using CTS or LIN mode in the near future, so this is placeholder code for the future
    }
    if (handle->SR & USART_SR_LBD)
    {
        handle->SR &= ~USART_SR_LBD; // We currently do not plan on using CTS or LIN mode in the near future, so this is placeholder code for the future
    }
    if (handle->SR & USART_SR_ORE)
    {
        // Communicate these errors to the user, so we can clear this bit but leave USART clear for future transactions
        if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._tx_busy)
        {
            active_uarts[idx].active_handle->tx_errors.overrun = 1;
        }
        else if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._rx_busy)
        {
            active_uarts[idx].active_handle->rx_errors.overrun = 1;
        }
        handle->SR &= ~USART_SR_ORE; // Clear Overrun Error interrupt flag
    }
    if (handle->SR & USART_SR_NE)
    {
        // Communicate these errors to the user, so we can clear this bit for future transactions
        if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._tx_busy)
        {
            active_uarts[idx].active_handle->tx_errors.noise_detected = 1;
        }
        else if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._rx_busy)
        {
            active_uarts[idx].active_handle->rx_errors.noise_detected = 1;
        }
        handle->SR &= ~USART_SR_NE; // Clear Noise Error (NF) interrupt flag
    }
    if (handle->SR & USART_SR_FE)
    {
        // Communicate these errors to the user, so we can clear this bit for future transactions
        if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._tx_busy)
        {
            active_uarts[idx].active_handle->tx_errors.framing_error = 1;
        }
        else if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._rx_busy)
        {
            active_uarts[idx].active_handle->rx_errors.framing_error = 1;
        }
        handle->SR &= ~USART_SR_FE; // Clear Framing Error interrupt flag
    }
    if (handle->SR & USART_SR_PE)
    {
        // Communicate these errors to the user, so we can clear this bit for future transactions
        if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._tx_busy)
        {
            active_uarts[idx].active_handle->tx_errors.parity_error = 1;
        }
        else if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._rx_busy)
        {
            active_uarts[idx].active_handle->rx_errors.parity_error = 1;
        }
        handle->SR &= ~USART_SR_PE; // Clear Parity Error interrupt flag
    }
    if (handle->SR & USART_SR_TC)
    {
        if (active_uarts[idx].active_handle != 0 && active_uarts[idx]._tx_busy)
        {
            active_uarts[idx]._tx_busy = 0;
            // active_uarts[idx].active_handle->periph->CR1 &= ~USART_CR1_TE;
        }
        handle->SR &= ~USART_SR_TC; // Clear Transfer complete interrupt flag
    }
    if (handle->SR & USART_SR_RXNE)
    {
        handle->SR &= ~USART_SR_RXNE; // Clear RX Not Empty interrupt flag
    }
    if (handle->SR & USART_SR_IDLE)
    {
        handle->SR &= ~USART_SR_IDLE; // Clear Idle Line interrupt flag
    }
    #endif
}


void DMA1_Stream6_IRQHandler()
{
    handleDMAxComplete(USART2_ACTIVE_IDX, DMA1_Stream6_IRQn, USART_DMA_TX);
}  

void DMA1_Stream5_IRQHandler()
{
    handleDMAxComplete(USART2_ACTIVE_IDX, DMA1_Stream5_IRQn, USART_DMA_RX);
}  

void USART2_IRQHandler()
{
    handleUsartIRQ(USART2, USART2_ACTIVE_IDX);
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

        // case LPUART1_BASE:
        //     NVIC->ISER[2] |= 1U << (LPUART1_IRQn - 64);
        //     break;
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

        // case LPUART1_BASE:
        //     NVIC->ISER[2] &= ~(1U << (LPUART1_IRQn - 64));
        //     break;
    }
}