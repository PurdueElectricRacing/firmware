#include "common/phal_G4/usart/usart.h"
#include "common/phal_G4/usart/usart_priv.h"

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/gpio/gpio.h"

static const PHAL_USART_HwMap_t USART_MAP[NUM_USART] = {
    [USART1_IDX] = {
        .rcc_enable_rg  = &RCC->APB2ENR,   
        .rcc_enable_msk = RCC_APB2ENR_USART1EN,
        .periph         = USART1,          
        .dma            = DMA1,
        .irq            = USART1_IRQn,
        .tx_channel     = 7,               
        .tx_request     = DMA_REQUEST_USART1_TX,
        .rx_channel     = 5,               
        .rx_request     = DMA_REQUEST_USART1_RX,
    },
    [USART2_IDX] = {
        .rcc_enable_rg  = &RCC->APB1ENR1,  
        .rcc_enable_msk = RCC_APB1ENR1_USART2EN,
        .periph         = USART2,          
        .dma            = DMA1,
        .irq            = USART2_IRQn,
        .tx_channel     = 4,               
        .tx_request     = DMA_REQUEST_USART2_TX,
        .rx_channel     = 3,               
        .rx_request     = DMA_REQUEST_USART2_RX,
    },
    [USART3_IDX] = {
        .rcc_enable_rg  = &RCC->APB1ENR1,  
        .rcc_enable_msk = RCC_APB1ENR1_USART3EN,
        .periph         = USART3,          
        .dma            = DMA1,
        .irq            = USART3_IRQn,
        .tx_channel     = 2,               
        .tx_request     = DMA_REQUEST_USART3_TX,
        .rx_channel     = 1,               
        .rx_request     = DMA_REQUEST_USART3_RX,
    },
};


#define _DEF_USART_RXDMA_CONFIG(rx_addr_, priority_, USARTx, dma_num, channel_num, req_id) \
    { \
        .periph_addr = (uint32_t)&((USARTx)->RDR), \
        .mem_addr    = (uint32_t)(rx_addr_), \
        .tx_size     = 1, \
        .circular    = false, \
        .mem_inc     = true, \
        .periph_inc  = false, \
        .mem_to_mem  = false, \
        .priority    = (priority_), \
        .dir         = 0, \
        .mem_size    = DMA_SIZE_8BIT, \
        .periph_size = DMA_SIZE_8BIT, \
        .tx_isr_en   = true, \
        .mux_request = (req_id), \
        .channel_idx = (channel_num), \
        .periph      = DMA##dma_num, \
    }

    
#define _DEF_USART_TXDMA_CONFIG(tx_addr_, priority_, USARTx, dma_num, channel_num, req_id) \
    { \
        .periph_addr = (uint32_t)&((USARTx)->TDR), \
        .mem_addr    = (uint32_t)(tx_addr_), \
        .tx_size     = 1, \
        .circular    = false, \
        .mem_inc     = true, \
        .periph_inc  = false, \
        .mem_to_mem  = false, \
        .priority    = (priority_), \
        .dir         = 1, \
        .mem_size    = DMA_SIZE_8BIT, \
        .periph_size = DMA_SIZE_8BIT, \
        .tx_isr_en   = true, \
        .mux_request = (req_id), \
        .channel_idx = (channel_num), \
        .periph      = DMA##dma_num, \
    }
typedef enum {
    USART_DMA_TX,
    USART_DMA_RX
} usart_dma_mode_t;

typedef struct {
    PHAL_USART_Handle_t* active_handle; //!< USART handle provided on initialization
    uint8_t cont_rx; //!< Flag controlling RX rececption mode (once or continously)
    uint8_t _tx_busy; //!< Waiting on a transmission to finish
    volatile uint8_t _rx_busy; //!< Waiting on a reception to finish
    volatile uint32_t rxfer_size; //!< Size of data to receive over DMA
} usart_active_transfer_t;

// A global array to hold the state of each active USART peripheral.
volatile usart_active_transfer_t active_uarts[NUM_USART];

/**
 * @brief Initializes a USART peripheral on an STM32G4 microcontroller.
 *
 * This function configures the USART peripheral, calculates and sets the
 * baud rate, and enables the necessary clocks and DMA streams.
 *
 * @param handle Pointer to a usart_init_t struct containing the configuration.
 * @param clock_rate The clock frequency of the peripheral in Hz.
 * @return true if initialization is successful, false otherwise.
 */
bool PHAL_USART_init(PHAL_USART_Handle_t* handle, const uint32_t clock_rate) {
    uint32_t div;

    // Add Handle to active peripheral set for keeping track of activity.
    active_uarts[handle->usart_active_num].active_handle = handle;

    // Disable peripheral until properly configured
    handle->periph->CR1 &= ~USART_CR1_UE;

    // 
    int idx = usart_idx_from_periph(handle->periph);
    if (idx < 0) {
        return false;
    }
    const PHAL_USART_HwMap_t* row = &USART_MAP[idx];
    *row->rcc_enable_rg |= row->rcc_enable_msk;

    if (handle->ovsample == OV_16) {
        div                 = (fck + (handle->baud_rate / 2U)) / handle->baud_rate;
        handle->periph->BRR = div;
    } else {
        div                 = (2U * fck + (handle->baud_rate / 2U)) / handle->baud_rate;
        uint32_t mantissa   = div & 0xFFF0;
        uint32_t fraction   = (div & 0x000F) >> 1;
        handle->periph->BRR = mantissa | fraction;
    }

    // Set CR1 parameters
    handle->periph->CR1 = 0U;
    handle->periph->CR1 |= (handle->parity != PT_NONE) << USART_CR1_PCE_Pos;

    switch (handle->word_length) {
        case WORD_7:
            handle->periph->CR1 |= USART_CR1_M1; // bit 28 = 1, bit 12 = 0
            break;

        case WORD_9:
            handle->periph->CR1 |= USART_CR1_M0; // bit 12 = 1, bit 28 = 0
            break;

        default: // WL_8B
            // both bits 0
            break;
    }

    handle->periph->CR1 |= (handle->parity >> 2) << USART_CR1_PS_Pos;
    handle->periph->CR1 |= handle->ovsample << USART_CR1_OVER8_Pos;
    handle->periph->CR1 |= USART_CR1_RXNEIE | USART_CR1_IDLEIE;

    // Set CR2 parameters
    handle->periph->CR2 = 0U;
    handle->periph->CR2 |= (handle->address & 0xF) << USART_CR2_ADD_Pos;
    handle->periph->CR2 |= handle->stop_bits << USART_CR2_STOP_Pos;

    // Set CR3 parameters
    handle->periph->CR3 |= handle->obsample << USART_CR3_ONEBIT_Pos;

    // Enable peripheral for use
    handle->periph->CR1 |= USART_CR1_UE;

    // Blocking is currently not supported without DMA configuration
    if (!handle->rx_dma || !handle->tx_dma)
        return false;

    // Configure DMA
    if (!PHAL_DMA_init(handle->tx_dma) || !PHAL_DMA_init(handle->rx_dma)) {
        return false;
    }

    return true;
}

/**
 * @brief Starts a DMA-based transmission.
 *
 * @param handle Pointer to the USART handle.
 * @param data Pointer to the data to transmit.
 * @param len The length of the data in bytes.
 * @return true if the transfer was started, false otherwise.
 */
bool PHAL_USART_txDMA(PHAL_USART_Handle_t* handle, uint8_t* data, uint32_t len) {
    if (active_uarts[handle->usart_active_num].active_handle != handle)
        return false;

    // Ensure any RX data is not overwritten before continuing with transfer
    while (
        (active_uarts[handle->usart_active_num].active_handle->periph->ISR & USART_ISR_RXNE_RXFNE))
        ;

    // Enable the correct DMA interrupt for the G4
    if (PHAL_DMA_getPeriph(handle->tx_dma) == DMA1) {
        NVIC_EnableIRQ(DMA1_Channel1_IRQn + (PHAL_DMA_getChannelIdx(handle->tx_dma) - 1));
    } else if (PHAL_DMA_getPeriph(handle->tx_dma) == DMA2) {
        NVIC_EnableIRQ(DMA2_Channel1_IRQn + (PHAL_DMA_getChannelIdx(handle->tx_dma) - 1));
    } else {
        return false;
    }

    PHAL_DMA_stop(handle->tx_dma);

    PHAL_DMA_setLength(handle->tx_dma, len);
    PHAL_DMA_setMemAddress(handle->tx_dma, (uint32_t)data);

    PHAL_DMA_restart(handle->tx_dma);

    active_uarts[handle->usart_active_num]._tx_busy = 1;

    handle->periph->CR3 |= USART_CR3_DMAT;
    handle->periph->CR1 |= USART_CR1_TE;

    PHAL_DMA_start(handle->tx_dma);
    return true;
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
bool PHAL_USART_rxDMA(PHAL_USART_Handle_t* handle, uint8_t* data, uint32_t len, bool cont) {
    if (active_uarts[handle->usart_active_num].active_handle != handle) {
        return false;
    }

    active_uarts[handle->usart_active_num].cont_rx    = cont;
    active_uarts[handle->usart_active_num].rxfer_size = len;
    handle->periph->CR1 |= USART_CR1_RE;

    switch ((ptr_int)handle->periph) {
        case USART1_BASE:
            NVIC_EnableIRQ(USART1_IRQn);
            break;
        case USART2_BASE:
            NVIC_EnableIRQ(USART2_IRQn);
            break;
        case USART3_BASE:
            NVIC_EnableIRQ(USART3_IRQn);
            break;
        case UART4_BASE:
            NVIC_EnableIRQ(UART4_IRQn);
            break;
        case LPUART1_BASE:
            NVIC_EnableIRQ(LPUART1_IRQn);
            break;
        default:
            return false;
    }

    if (PHAL_DMA_getPeriph(handle->rx_dma) == DMA1) {
        NVIC_EnableIRQ(DMA1_Channel1_IRQn + (PHAL_DMA_getChannelIdx(handle->rx_dma) - 1));
    } else if (PHAL_DMA_getPeriph(handle->rx_dma) == DMA2) {
        NVIC_EnableIRQ(DMA2_Channel1_IRQn + (PHAL_DMA_getChannelIdx(handle->rx_dma) - 1));
    } else {
        return false;
    }

    PHAL_DMA_setMemAddress(handle->rx_dma, (uint32_t)data);
    handle->periph->CR3 |= USART_CR3_DMAR;

    PHAL_DMA_setLength(handle->rx_dma, len);
    PHAL_DMA_start(handle->rx_dma);

    return true;
}

/**
 * @brief Checks if a DMA transmission is currently busy.
 *
 * @param handle Pointer to the USART handle.
 * @return true if the peripheral is busy, false otherwise.
 */
volatile bool PHAL_USART_txBusy(usart_init_t* handle) {
    return active_uarts[handle->usart_active_num]._tx_busy;
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

    // USART RX Not Empty interrupt flag
    if (isr & USART_ISR_RXNE_RXFNE) {
        active_uarts[idx]._rx_busy = 1;
        PHAL_DMA_setLength(active_uarts[idx].active_handle->rx_dma,
                                active_uarts[idx].rxfer_size);
        PHAL_DMA_restart(active_uarts[idx].active_handle->rx_dma);
        // Read RDR to clear RXNE flag if set
        (void)active_uarts[idx].active_handle->periph->RDR;
        active_uarts[idx].active_handle->periph->RQR = USART_RQR_RXFRQ;
        active_uarts[idx].active_handle->periph->CR1 &= ~USART_CR1_RXNEIE;
        // Clear any errors that may have been set in the previous Rx
        active_uarts[idx].active_handle->rx_errors.framing_error  = 0;
        active_uarts[idx].active_handle->rx_errors.noise_detected = 0;
        active_uarts[idx].active_handle->rx_errors.overrun        = 0;
        active_uarts[idx].active_handle->rx_errors.parity_error   = 0;
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
        PHAL_DMA_stop(active_uarts[idx].active_handle->rx_dma);
        if (active_uarts[idx].cont_rx) {
            // Read RDR to clear RXNE before re-enabling RXNEIE
            if (periph->ISR & USART_ISR_RXNE_RXFNE) {
                (void)periph->RDR;
            }
            active_uarts[idx].active_handle->periph->CR1 |= USART_CR1_RXNEIE;
        } else {
            active_uarts[idx].active_handle->periph->CR1 &= ~USART_CR1_RE;
        }
        active_uarts[idx]._rx_busy = 0;
        periph->ICR |= USART_ICR_IDLECF;
        usart_receive_complete_callback(active_uarts[idx].active_handle);
    }
}

/**
 * @brief Handles DMA interrupts for a specific channel.
 *
 * This function checks for transfer complete and error flags on the DMA channel
 * and handles the post-transfer cleanup.
 *
 * @param dma_periph The DMA controller instance (DMA1 or DMA2).
 * @param channel The channel number (1-8).
 * @param dma_type The DMA transfer type (TX or RX).
 * @param idx The index of the USART in the active_uarts array.
 */
static void
handleDMAxComplete(DMA_TypeDef* dma_periph, uint8_t channel, uint8_t dma_type, uint8_t idx) {
    // The bit masks for each channel's flags
    uint32_t tcif_mask = DMA_ISR_TCIF1 << (4 * (channel - 1));
    uint32_t teif_mask = DMA_ISR_TEIF1 << (4 * (channel - 1));
    uint32_t htif_mask = DMA_ISR_HTIF1 << (4 * (channel - 1));
    uint32_t gif_mask  = DMA_ISR_GIF1 << (4 * (channel - 1));

    // Check for a Transfer Complete interrupt
    if (dma_periph->ISR & tcif_mask) {
        // Clear the transfer complete flag
        dma_periph->IFCR |= tcif_mask;

        if (dma_type == USART_DMA_TX) {
            PHAL_DMA_stop(active_uarts[idx].active_handle->tx_dma);
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

__WEAK void PHAL_USART_rxCallback(PHAL_USART_Handle_t *handle) { }

/* DMA Interrupt Handlers */
__attribute__((weak)) void DMA1_Channel7_IRQHandler(void) {
    handleDMAxComplete(DMA1, 7, USART_DMA_TX, USART1_ACTIVE_IDX);
}

__attribute__((weak)) void DMA1_Channel4_IRQHandler(void) {
    handleDMAxComplete(DMA1, 4, USART_DMA_TX, USART2_ACTIVE_IDX);
}

__attribute__((weak)) void DMA1_Channel2_IRQHandler(void) {
    handleDMAxComplete(DMA1, 2, USART_DMA_TX, USART3_ACTIVE_IDX);
}

/// Service USART3 RX when it owns DMA1 channel 1.
void PHAL_USART_DMA1_Channel1_IRQHandler(void) {
    handleDMAxComplete(DMA1, 1, USART_DMA_RX, USART3_ACTIVE_IDX);
}

// ADC provides the strong shared vector when linked and delegates here when
// ADC1 does not own the channel. This weak vector covers USART-only builds.
__attribute__((weak)) void DMA1_Channel1_IRQHandler(void) {
    PHAL_USART_DMA1_Channel1_IRQHandler();
}

void DMA2_Channel7_IRQHandler(void) {
    handleDMAxComplete(DMA2, 7, USART_DMA_TX, LPUART1_ACTIVE_IDX);
}

void DMA2_Channel6_IRQHandler(void) {
    handleDMAxComplete(DMA2, 6, USART_DMA_RX, LPUART1_ACTIVE_IDX);
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
