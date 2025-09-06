#include "common/phal_G4/usart/usart.h" 

// These items should not be used/modified by anybody other than the HAL
typedef enum {
    USART_DMA_TX, //!< USART is transmitting over DMA
    USART_DMA_RX  //!< USART is receiving over DMA
} usart_dma_mode_t;

typedef struct {
    usart_init_t* active_handle; //!< USART handle provided on initialization
    uint8_t          cont_rx;       //!< Flag controlling RX rececption mode (once or continously)
    uint8_t          _tx_busy;      //!< Waiting on a transmission to finish
    volatile uint8_t _rx_busy;      //!< Waiting on a reception to finish
    volatile uint32_t rxfer_size;   //!< Size of data to receive over DMA
} usart_active_transfer_t;

// A global array to hold the state of each active USART peripheral.
volatile usart_active_transfer_t active_uarts[TOTAL_NUM_UART];

/**
 * @brief Initializes a USART peripheral on an STM32G4 microcontroller.
 *
 * This function configures the USART peripheral, calculates and sets the
 * baud rate, and enables the necessary clocks and DMA streams.
 *
 * @param handle Pointer to a usart_init_t struct containing the configuration.
 * @param fck The clock frequency of the peripheral in Hz.
 * @return true if initialization is successful, false otherwise.
 */
bool PHAL_initUSART(usart_init_t* handle, const uint32_t fck) {
    uint32_t div;

    // Add Handle to active peripheral set for keeping track of activity.
    active_uarts[handle->usart_active_num].active_handle = handle;

    // Disable peripheral until properly configured
    handle->periph->CR1 &= ~USART_CR1_UE;

    // Enable peripheral clock in RCC for G4.
    switch ((ptr_int)handle->periph) {
        case USART1_BASE:
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            break;
        case USART2_BASE:
            RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
            break;
        case USART3_BASE:
            RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
            break;
        case UART4_BASE:
            RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;
            break;
        case LPUART1_BASE:
            RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
            break;
        default:
            return false;
    }

    if (handle->ovsample == OV_16) {
        div = (fck + (handle->baud_rate / 2U)) / handle->baud_rate;
        handle->periph->BRR = div;
    } else {
        div = (2U * fck + (handle->baud_rate / 2U)) / handle->baud_rate;
        uint32_t mantissa = div & 0xFFF0;
        uint32_t fraction = (div & 0x000F) >> 1;
        handle->periph->BRR = mantissa | fraction;
    }

    // Set CR1 parameters
    handle->periph->CR1 = 0U;
    handle->periph->CR1 |= (handle->parity != PT_NONE) << USART_CR1_PCE_Pos;
    handle->periph->CR1 |= handle->word_length << USART_CR1_M_Pos;
    handle->periph->CR1 |= (handle->parity >> 2) << USART_CR1_PS_Pos;
    handle->periph->CR1 |= handle->ovsample << USART_CR1_OVER8_Pos;
    handle->periph->CR1 |= USART_CR1_RXNEIE | USART_CR1_IDLEIE;
    handle->periph->CR1 |= USART_CR1_TXEIE;

    // Set CR2 parameters
    handle->periph->CR2 = 0U;
    handle->periph->CR2 |= (handle->address & 0xF) << USART_CR2_ADD_Pos;
    handle->periph->CR2 |= handle->stop_bits << USART_CR2_STOP_Pos;

    // Set CR3 parameters
    handle->periph->CR3 |= handle->obsample << USART_CR3_ONEBIT_Pos;

    // Enable peripheral for use
    handle->periph->CR1 |= USART_CR1_UE;

    // Blocking is currently not supported without DMA configuration
    if (!handle->rx_dma_cfg || !handle->tx_dma_cfg)
        return false;

    // Configure DMA
    if (!PHAL_initDMA(handle->tx_dma_cfg) || !PHAL_initDMA(handle->rx_dma_cfg)) {
        return false;
    }

    return true;
}

/**
 * @brief Transmits data in blocking mode.
 *
 * @param handle Pointer to the USART handle.
 * @param data Pointer to the data to transmit.
 * @param len The length of the data in bytes.
 */
void PHAL_usartTxBl(usart_init_t* handle, uint8_t* data, uint32_t len) {
    handle->periph->CR1 |= USART_CR1_TE;
    for (int i = 0; i < len; i++) {
        while (!(handle->periph->ISR & USART_ISR_TXE_TXFNF))
            ;
        handle->periph->TDR = data[i] & 0xff;
    }
    while (!(handle->periph->ISR & USART_ISR_TC))
        ;
}

/**
 * @brief Receives data in blocking mode.
 *
 * @param handle Pointer to the USART handle.
 * @param data Pointer to the buffer to store received data.
 * @param len The length of the data to receive in bytes.
 */
void PHAL_usartRxBl(usart_init_t* handle, uint8_t* data, uint32_t len) {
    handle->periph->CR1 |= USART_CR1_RE;
    for (int i = 0; i < len; i++) {
        while (!(handle->periph->ISR & USART_ISR_RXNE_RXFNE))
            ;
        data[i] = handle->periph->RDR & 0xff;
    }
}

/**
 * @brief Starts a DMA-based transmission.
 *
 * @param handle Pointer to the USART handle.
 * @param data Pointer to the data to transmit.
 * @param len The length of the data in bytes.
 * @return true if the transfer was started, false otherwise.
 */
bool PHAL_usartTxDma(usart_init_t* handle, uint8_t* data, uint32_t len) {
    if (active_uarts[handle->usart_active_num].active_handle != handle)
        return false;

    // Ensure any RX data is not overwritten before continuing with transfer
    while ((active_uarts[handle->usart_active_num].active_handle->periph->ISR & USART_ISR_RXNE_RXFNE))
        ;

    // Enable the correct DMA interrupt for the G4
    // NOTE: Verify these mappings in the G4 Reference Manual
    switch ((ptr_int)handle->periph) {
        case USART1_BASE:
            NVIC_EnableIRQ(DMA1_Channel7_IRQn);
            break;
        case USART2_BASE:
            NVIC_EnableIRQ(DMA1_Channel7_IRQn);
            break;
        case USART3_BASE:
            NVIC_EnableIRQ(DMA1_Channel2_IRQn);
            break;
        case UART4_BASE:
            NVIC_EnableIRQ(DMA2_Channel3_IRQn);
            break;
        case LPUART1_BASE:
            NVIC_EnableIRQ(DMA2_Channel7_IRQn);
            break;
        default:
            return false;
    }

    PHAL_stopTxfer(handle->tx_dma_cfg);

    PHAL_DMA_setTxferLength(handle->tx_dma_cfg, len);
    PHAL_DMA_setMemAddress(handle->tx_dma_cfg, (uint32_t)data);

    active_uarts[handle->usart_active_num]._tx_busy = 1;

    handle->periph->CR3 |= USART_CR3_DMAT;
    handle->periph->CR1 |= USART_CR1_TE;
    
    PHAL_startTxfer(handle->tx_dma_cfg);
    return true;
}

/**
 * @brief Checks if a DMA transmission is currently busy.
 *
 * @param handle Pointer to the USART handle.
 * @return true if the peripheral is busy, false otherwise.
 */
volatile bool PHAL_usartTxBusy(usart_init_t* handle) {
    return active_uarts[handle->usart_active_num]._tx_busy;
}

/**
 * @brief Starts a DMA-based reception.
 *
 * @param handle Pointer to the USART handle.
 * @param data Pointer to the buffer to store received data.
 * @param len The length of the data to receive in bytes.
 * @param cont true for continuous reception, false for single reception.
 * @return true if the transfer was started, false otherwise.
 */
bool PHAL_usartRxDma(usart_init_t* handle, uint16_t* data, uint32_t len, bool cont) {
    if (active_uarts[handle->usart_active_num].active_handle != handle)
        return false;

    active_uarts[handle->usart_active_num].cont_rx = cont;
    active_uarts[handle->usart_active_num].rxfer_size = len;
    handle->periph->CR1 |= USART_CR1_RE;

    // Enable the correct DMA and USART interrupts for the G4
    // NOTE: Verify these mappings in the G4 Reference Manual
    switch ((ptr_int)handle->periph) {
        case USART1_BASE:
            NVIC_EnableIRQ(DMA1_Channel5_IRQn);
            NVIC_EnableIRQ(USART1_IRQn);
            break;
        case USART2_BASE:
            NVIC_EnableIRQ(DMA1_Channel6_IRQn);
            NVIC_EnableIRQ(USART2_IRQn);
            break;
        case USART3_BASE:
            NVIC_EnableIRQ(DMA1_Channel3_IRQn);
            NVIC_EnableIRQ(USART3_IRQn);
            break;
        case UART4_BASE:
            NVIC_EnableIRQ(DMA2_Channel5_IRQn);
            NVIC_EnableIRQ(UART4_IRQn);
            break;
        case LPUART1_BASE:
            NVIC_EnableIRQ(DMA2_Channel6_IRQn);
            NVIC_EnableIRQ(LPUART1_IRQn);
            break;
        default:
            return false;
    }

    PHAL_DMA_setMemAddress(handle->rx_dma_cfg, (uint32_t)data);
    handle->periph->CR3 |= USART_CR3_DMAR;
    return true;
}

/**
 * @brief Disables continuous DMA reception.
 *
 * @param handle Pointer to the USART handle.
 * @return true if the operation was successful, false otherwise.
 */
bool PHAL_disableContinousRxDMA(usart_init_t* handle) {
    if (active_uarts[handle->usart_active_num].active_handle != handle)
        return false;
        
    active_uarts[handle->usart_active_num].cont_rx = 0;
    handle->periph->CR1 &= ~USART_CR1_RE;
    handle->periph->CR3 &= ~USART_CR3_DMAR;

    // Disable the correct DMA and USART interrupts for the G4
    // NOTE: Verify these mappings in the G4 Reference Manual
    switch ((ptr_int)handle->periph) {
        case USART1_BASE:
            NVIC_DisableIRQ(DMA1_Channel5_IRQn);
            NVIC_DisableIRQ(USART1_IRQn);
            break;
        case USART2_BASE:
            NVIC_DisableIRQ(DMA1_Channel6_IRQn);
            NVIC_DisableIRQ(USART2_IRQn);
            break;
        case USART3_BASE:
            NVIC_DisableIRQ(DMA1_Channel3_IRQn);
            NVIC_DisableIRQ(USART3_IRQn);
            break;
        case UART4_BASE:
            NVIC_DisableIRQ(DMA2_Channel5_IRQn);
            NVIC_DisableIRQ(UART4_IRQn);
            break;
        case LPUART1_BASE:
            NVIC_DisableIRQ(DMA2_Channel6_IRQn);
            NVIC_DisableIRQ(LPUART1_IRQn);
            break;
        default:
            return false;
    }

    active_uarts[handle->usart_active_num]._rx_busy = 0;
    return true;
}

/**
 * @brief Checks if a DMA reception is currently busy.
 *
 * @param handle Pointer to the USART handle.
 * @return true if the peripheral is busy, false otherwise.
 */
bool PHAL_usartRxBusy(usart_init_t* handle) {
    return active_uarts[handle->usart_active_num]._rx_busy;
}

/**
 * @brief Handles the USART interrupt logic.
 *
 * This function checks for various USART flags and manages DMA transfers and
 * error handling.
 *
 * @param periph The USART peripheral instance.
 * @param idx The index of the USART in the active_uarts array.
 */
static void handleUsartIRQ(USART_TypeDef* periph, uint8_t idx) {
    uint32_t isr = periph->ISR;
    static uint32_t trash;

    // USART RX Not Empty interrupt flag
    if (isr & USART_ISR_RXNE_RXFNE) {
        active_uarts[idx]._rx_busy = 1;
        PHAL_DMA_setTxferLength(active_uarts[idx].active_handle->rx_dma_cfg, active_uarts[idx].rxfer_size);
        PHAL_reEnable(active_uarts[idx].active_handle->rx_dma_cfg);
        active_uarts[idx].active_handle->periph->RQR = USART_RQR_RXFRQ;
        active_uarts[idx].active_handle->periph->CR1 &= ~USART_CR1_RXNEIE;
        // Clear any errors that may have been set in the previous Rx
        active_uarts[idx].active_handle->rx_errors.framing_error = 0;
        active_uarts[idx].active_handle->rx_errors.noise_detected = 0;
        active_uarts[idx].active_handle->rx_errors.overrun = 0;
        active_uarts[idx].active_handle->rx_errors.parity_error = 0;
    }
    
    // Overrun Error Flag
    if (isr & USART_ISR_ORE) {
        active_uarts[idx].active_handle->rx_errors.overrun = 1;
        periph->ICR |= USART_ICR_ORECF;
    }
    // Noise Error Flag
    if (isr & USART_ISR_NE) {
        active_uarts[idx].active_handle->rx_errors.noise_detected = 1;
        periph->ICR |= USART_ICR_NECF;
    }
    // Framing Error Flag
    if (isr & USART_ISR_FE) {
        active_uarts[idx].active_handle->rx_errors.framing_error = 1;
        periph->ICR |= USART_ICR_FECF;
    }
    // Parity Error Flag
    if (isr & USART_ISR_PE) {
        active_uarts[idx].active_handle->rx_errors.parity_error = 1;
        periph->ICR |= USART_ICR_PECF;
    }

    // Idle line detected
    if (isr & USART_ISR_IDLE) {
        PHAL_stopTxfer(active_uarts[idx].active_handle->rx_dma_cfg);
        if (active_uarts[idx].cont_rx) {
            active_uarts[idx].active_handle->periph->CR1 |= USART_CR1_RXNEIE;
        } else {
            active_uarts[idx].active_handle->periph->CR1 &= ~USART_CR1_RE;
        }
        active_uarts[idx]._rx_busy = 0;
        periph->ICR |= USART_ICR_IDLECF;
        usart_recieve_complete_callback(active_uarts[idx].active_handle);
    }
}

/**
 * @brief Handles DMA interrupts for a specific channel.
 *
 * This function checks for transfer complete and error flags on the DMA channel
 * and handles the post-transfer cleanup.
 *
 * @param dma_periph The DMA controller instance (DMA1 or DMA2).
 * @param channel The channel number (0-7).
 * @param dma_type The DMA transfer type (TX or RX).
 * @param idx The index of the USART in the active_uarts array.
 */
static void handleDMAxComplete(DMA_TypeDef* dma_periph, uint8_t channel, uint8_t dma_type, uint8_t idx) {
    // The bit masks for each channel's flags
    uint32_t tcif_mask = DMA_ISR_TCIF1 << (4 * channel);
    uint32_t teif_mask = DMA_ISR_TEIF1 << (4 * channel);
    uint32_t htif_mask = DMA_ISR_HTIF1 << (4 * channel);
    uint32_t gif_mask  = DMA_ISR_GIF1 << (4 * channel);

    // Check for a Transfer Complete interrupt
    if (dma_periph->ISR & tcif_mask) {
        // Clear the transfer complete flag
        dma_periph->IFCR |= tcif_mask;
        
        if (dma_type == USART_DMA_TX) {
            PHAL_stopTxfer(active_uarts[idx].active_handle->tx_dma_cfg);
            active_uarts[idx]._tx_busy = 0;
        }
    }

    // Check for a Transfer Error interrupt
    if (dma_periph->ISR & teif_mask) {
        // Clear the transfer error flag
        dma_periph->IFCR |= teif_mask;
        
        if (dma_type == USART_DMA_TX) {
            active_uarts[idx].active_handle->tx_errors.dma_transfer_error = 1;
        } else {
            active_uarts[idx].active_handle->rx_errors.dma_transfer_error = 1;
        }
    }
    
    // Check for Half Transfer Complete interrupt (if enabled)
    if (dma_periph->ISR & htif_mask) {
        dma_periph->IFCR |= htif_mask;
    }

    // Clear any other global flags for this channel
    dma_periph->IFCR |= gif_mask;
}

__WEAK void usart_recieve_complete_callback(usart_init_t* handle) {
    return;
}

/* DMA Interrupt Handlers */
void DMA1_Channel7_IRQHandler(void) {
    handleDMAxComplete(DMA1, 7, USART_DMA_TX, USART1_ACTIVE_IDX); // Assumes USART1 TX
    handleDMAxComplete(DMA1, 7, USART_DMA_TX, USART2_ACTIVE_IDX); // Assumes USART2 TX
}
void DMA1_Channel5_IRQHandler(void) {
    handleDMAxComplete(DMA1, 5, USART_DMA_RX, USART1_ACTIVE_IDX); // Assumes USART1 RX
}
void DMA1_Channel6_IRQHandler(void) {
    handleDMAxComplete(DMA1, 6, USART_DMA_RX, USART2_ACTIVE_IDX); // Assumes USART2 RX
}
void DMA1_Channel2_IRQHandler(void) {
    handleDMAxComplete(DMA1, 2, USART_DMA_TX, USART3_ACTIVE_IDX); // Assumes USART3 TX
}
void DMA1_Channel3_IRQHandler(void) {
    handleDMAxComplete(DMA1, 3, USART_DMA_RX, USART3_ACTIVE_IDX); // Assumes USART3 RX
}
void DMA2_Channel7_IRQHandler(void) {
    handleDMAxComplete(DMA2, 7, USART_DMA_TX, LPUART1_ACTIVE_IDX); // Assumes LPUART1 TX
}
void DMA2_Channel6_IRQHandler(void) {
    handleDMAxComplete(DMA2, 6, USART_DMA_RX, LPUART1_ACTIVE_IDX); // Assumes LPUART1 RX
}

/* USART Interrupt Handlers */
void USART1_IRQHandler(void) {
    handleUsartIRQ(USART1, USART1_ACTIVE_IDX);
}
void USART2_IRQHandler(void) {
    handleUsartIRQ(USART2, USART2_ACTIVE_IDX);
}
void USART3_IRQHandler(void) {
    handleUsartIRQ(USART3, USART3_ACTIVE_IDX);
}
void LPUART1_IRQHandler(void) {
    handleUsartIRQ(LPUART1, LPUART1_ACTIVE_IDX);
}
