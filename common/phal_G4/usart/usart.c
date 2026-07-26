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
        .tx_dma_irq     = DMA1_Channel7_IRQn,
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
        .tx_dma_irq     = DMA1_Channel4_IRQn,
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
        .tx_dma_irq     = DMA1_Channel2_IRQn,
        .irq            = USART3_IRQn,
        .tx_channel     = 2,               
        .tx_request     = DMA_REQUEST_USART3_TX,
        .rx_channel     = 1,               
        .rx_request     = DMA_REQUEST_USART3_RX,
    },
};

typedef struct {
    PHAL_USART_Handle_t*    handle; //!< USART handle provided on initialization
    dma_init_t              tx_dma; //!< USART transfer DMA configs
    dma_init_t              rx_dma; //!< USART receive DMA configs
    volatile uint32_t       rxfer_size; //!< Size of data to receive over DMA
    uint8_t                 tx_busy; //!< Waiting on a transmission to finish
    uint8_t                 cont_rx; //!< Flag controlling RX rececption mode (once or continously)
} PHAL_USART_state_t;

PHAL_USART_state_t usart_state[NUM_USART];

static int usart_idx_from_periph(USART_TypeDef *periph) {
    for (uint8_t i = 0; i < NUM_USART; i++) {
        if (USART_MAP[i].periph == periph) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Initializes a USART peripheral on an STM32G4 microcontroller.
 *
 * This function configures the USART peripheral, calculates and sets the
 * baud rate, and enables the necessary clocks and DMA streams.
 *
 * @param handle Pointer to a PHAL_USART_Handle_t struct containing the configuration.
 * @param clock_rate The clock frequency of the peripheral in Hz.
 * @return true if initialization is successful, false otherwise.
 */
bool PHAL_USART_init(PHAL_USART_Handle_t* handle, const uint32_t clock_rate) {
    int active_idx = usart_idx_from_periph(handle->periph);
    if (active_idx < 0) return false;
    const PHAL_USART_HwMap_t* usart_map = &USART_MAP[active_idx];

    // Add Handle to active peripheral set for keeping track of activity.
    usart_state[active_idx].handle = handle;

    // Enable USART Register
    *usart_map->rcc_enable_rg |= usart_map->rcc_enable_msk;

    // Reset USART Control Register
    handle->periph->CR1 = 0U;

    // Set Baud Rate
    handle->periph->BRR = (clock_rate + (handle->baud_rate / 2U)) / handle->baud_rate;

    // By default, USART set to 8 bit, oversampling by 16
    // and Parity control disabled

    // IDLE interrupt enabled
    handle->periph->CR1 |= USART_CR1_IDLEIE;

    // Enable the USART interrupt 
    NVIC_EnableIRQ(usart_map->irq);

    // Enable the TX DMA transfer-complete interrupt 
    NVIC_EnableIRQ(usart_map->tx_dma_irq);

    // Reset Control Register 2
    handle->periph->CR2 = 0U;

    // Reset Control Register 3
    handle->periph->CR3 = 0U;

    // Enable peripheral for use
    handle->periph->CR1 |= USART_CR1_UE;

    // TX DMA Init
    usart_state[active_idx].tx_dma = (dma_init_t) {
        .periph_addr = (uint32_t)&handle->periph->TDR,
        .periph = usart_map->dma,
        .channel_idx = usart_map->tx_channel,
        .mux_request = usart_map->tx_request,
        .mem_size = DMA_SIZE_8BIT,
        .periph_size = DMA_SIZE_8BIT,
        .mem_inc = true,
        .tx_isr_en = true,
        .dir = 1,
        .priority = 1
    };

    // RX DMA Init
    usart_state[active_idx].rx_dma = (dma_init_t) {
        .periph_addr = (uint32_t)&handle->periph->RDR,
        .periph = usart_map->dma,
        .channel_idx = usart_map->rx_channel,
        .mux_request = usart_map->rx_request,
        .mem_size = DMA_SIZE_8BIT,
        .periph_size = DMA_SIZE_8BIT,
        .mem_inc = true,
        .dir = 0,
        .priority = 2
    };

    // Configure DMA
    if (!PHAL_initDMA(&usart_state[active_idx].tx_dma) || !PHAL_initDMA(&usart_state[active_idx].rx_dma)) {
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
    int active_idx = usart_idx_from_periph(handle->periph);

    if (active_idx < 0) return false;
    if (usart_state[active_idx].handle != handle) return false;

    // Allows software to know transfer is in progress
    usart_state[active_idx].tx_busy = 1;

    // Allows USART to request a new byte from DMA
    // once its ready. This enables that DMA request
    handle->periph->CR3 |= USART_CR3_DMAT;

    // Enable USART transmitter
    handle->periph->CR1 |= USART_CR1_TE;

    dma_init_t *tx_dma = &usart_state[active_idx].tx_dma;

    // Configure DMA transfer (channel must be disabled to set length/address)
    PHAL_stopTxfer(tx_dma);
    PHAL_DMA_setTxferLength(tx_dma, len);
    PHAL_DMA_setMemAddress(tx_dma, (uint32_t)data);

    // reEnable clears stale channel flags and re-enables the channel (starts the transfer)
    PHAL_reEnable(tx_dma);

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
    int active_idx = usart_idx_from_periph(handle->periph);
    
    if (active_idx < 0) return false;
    if (usart_state[active_idx].handle != handle) return false;

    usart_state[active_idx].cont_rx = cont;
    usart_state[active_idx].rxfer_size = len;

    // Enable USART receiver
    // USART listens to that RX pin
    handle->periph->CR1 |= USART_CR1_RE;

    // Enable DMA receiver
    // Automatically moves bytes into buffer
    handle->periph->CR3 |= USART_CR3_DMAR;

    dma_init_t *rx_dma = &usart_state[active_idx].rx_dma;

    // Set buffer address for receiving bytes
    PHAL_DMA_setMemAddress(rx_dma, (uint32_t)data);

    // Tells DMA how many bytes to move before stopping
    PHAL_DMA_setTxferLength(rx_dma, len);

    PHAL_startTxfer(rx_dma);

    return true;
}

/**
 * @brief Checks if a DMA transmission is currently busy.
 *
 * @param handle Pointer to the USART handle.
 * @return true if the peripheral is busy, false otherwise.
 */
bool PHAL_USART_txBusy(PHAL_USART_Handle_t* handle) {
    int active_idx = usart_idx_from_periph(handle->periph);
    if (active_idx < 0) return false;
    return usart_state[active_idx].tx_busy;
}

/**
 * @brief Handles the USART interrupt logic.
 *
 * This function checks for various USART 
 * flags and manages DMA transfers and
 * error handling.
 *
 * @param active_idx The index of the USART in the usart_state array.
 */
static void PHAL_USART_HandleIRQ(uint8_t active_idx) {
    USART_TypeDef* periph = USART_MAP[active_idx].periph;
    uint32_t isr = periph->ISR;

    // Idle line detected
    if (isr & USART_ISR_IDLE) {
        dma_init_t* rx_dma = &usart_state[active_idx].rx_dma;
        PHAL_stopTxfer(rx_dma);
        
        if (usart_state[active_idx].cont_rx) {
            // Rearm because by now, transfer length had counted down to 0
            PHAL_DMA_setTxferLength(rx_dma, usart_state[active_idx].rxfer_size);
            PHAL_startTxfer(rx_dma);
        } else {
            periph->CR1 &= ~USART_CR1_RE;
        }
        
        PHAL_USART_rxCallback(usart_state[active_idx].handle);
    }

    // ICR is write to clear
    // Clear all error flags unconditionally
    periph->ICR = USART_ICR_IDLECF | 
        USART_ICR_ORECF | 
        USART_ICR_FECF | 
        USART_ICR_PECF | 
        USART_ICR_NECF;
}

/**
 * @brief Handles DMA interrupts for a specific channel.
 *
 * This function checks for transfer complete on the 
 * DMA channel and handles the post-transfer cleanup.
 *
 * @param channel The channel number (1-8).
 * @param idx The index of the USART in the usart_state array.
 */
static void PHAL_USART_HandleDMA(uint8_t channel, uint8_t idx) {
    DMA_TypeDef* dma_periph = USART_MAP[idx].dma;
    uint32_t shift = 4 * (channel - 1);

    // If DMA interrupt active
    if (dma_periph->ISR & (DMA_ISR_TCIF1 << shift)) {
        // Clear channel EN (enable) bit
        PHAL_stopTxfer(&usart_state[idx].tx_dma);
        // This handle fires when interrupt complete,
        // so mark it as free
        usart_state[idx].tx_busy = 0;
    }

    // Clear all interrupt flags for this channel (CGIF clears TCIF/HTIF/TEIF/GIF)
    dma_periph->IFCR = DMA_IFCR_CGIF1 << shift;
}

__WEAK void PHAL_USART_rxCallback(PHAL_USART_Handle_t *handle) { }

/* DMA Interrupt Handlers */
__attribute__((weak)) void DMA1_Channel7_IRQHandler(void) {
    PHAL_USART_HandleDMA( 7, USART1_IDX);
}

__attribute__((weak)) void DMA1_Channel4_IRQHandler(void) {
    PHAL_USART_HandleDMA( 4, USART2_IDX);
}

__attribute__((weak)) void DMA1_Channel2_IRQHandler(void) {
    PHAL_USART_HandleDMA( 2, USART3_IDX);
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
    PHAL_USART_HandleIRQ(USART1_IDX);
}

void USART2_IRQHandler(void) {
    PHAL_USART_HandleIRQ(USART2_IDX);
}

void USART3_IRQHandler(void) {
    PHAL_USART_HandleIRQ(USART3_IDX);
}
