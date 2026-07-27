#include "common/phal_G4/usart/usart.h"
#include "common/phal_G4/usart/usart_priv.h"

#include "common/phal_G4/dma/dma.h"

typedef struct {
    PHAL_USART_Handle_t *handle; //!< handle registered at init
    dma_init_t tx_dma;           //!< TX DMA descriptor (built in init)
    dma_init_t rx_dma;           //!< RX DMA descriptor (built in init)
    volatile uint32_t rxfer_size; //!< configured RX length (for continuous re-arm)
    volatile bool tx_busy;       //!< set when a TX is in flight, cleared by the DMA ISR
    bool cont_rx;                //!< continuous vs one-shot reception
} PHAL_USART_state_t;

static PHAL_USART_state_t usart_state[NUM_USART];

/**
 * @brief Initialize a USART peripheral for DMA-driven communication.
 *
 * Enables the peripheral clock and applies a fixed frame format of 8 data bits,
 * no parity, 1 stop bit (8N1) with 16x oversampling and no hardware flow control.
 * Derives the baud-rate divisor from @p clock_rate, enables the IDLE-line
 * interrupt (used to signal RX frame completion) and the TX DMA transfer-complete
 * interrupt, then initializes the TX and RX DMA channels.
 *
 * Call once per USART before any tx/rx. The USART GPIO pins must already be
 * configured by the caller.
 *
 * @param handle Handle identifying the peripheral and desired baud rate
 * @param clock_rate Frequency (Hz) of the bus clock feeding this USART (APB1/APB2)
 * @return true on success, false if the peripheral is unsupported or DMA init failed
 */
bool PHAL_USART_init(PHAL_USART_Handle_t *handle, const uint32_t clock_rate) {
    ssize_t idx = USART_PRIV_idx_from_periph(handle->periph);
    if (idx < 0) return false;

    // Register the handle so the interrupt handlers can reach it.
    usart_state[idx].handle = handle;

    USART_PRIV_configure(idx, handle->baud_rate, clock_rate);
    USART_PRIV_build_dma(idx, &usart_state[idx].tx_dma, &usart_state[idx].rx_dma);

    if (!PHAL_initDMA(&usart_state[idx].tx_dma) || !PHAL_initDMA(&usart_state[idx].rx_dma)) {
        return false;
    }

    return true;
}

/**
 * @brief Start a DMA-based transmission.
 *
 * @param handle Handle of the USART to transmit on
 * @param data Buffer to send
 * @param len Number of bytes to send
 * @return true if the transfer started, false otherwise
 */
bool PHAL_USART_txDMA(PHAL_USART_Handle_t *handle, uint8_t *data, uint32_t len) {
    ssize_t idx = USART_PRIV_idx_from_periph(handle->periph);
    if (idx < 0) return false;
    if (usart_state[idx].handle != handle) return false;

    usart_state[idx].tx_busy = true;
    USART_PRIV_start_tx(handle->periph);

    // Re-target the TX channel at this buffer (channel must be disabled to set
    // length/address); reEnable clears stale flags and starts the transfer.
    dma_init_t *tx_dma = &usart_state[idx].tx_dma;
    PHAL_stopTxfer(tx_dma);
    PHAL_DMA_setTxferLength(tx_dma, len);
    PHAL_DMA_setMemAddress(tx_dma, (uint32_t)data);
    PHAL_reEnable(tx_dma);

    return true;
}

/**
 * @brief Start a DMA-based reception, completed on the IDLE line.
 *
 * @param handle Handle of the USART to receive on
 * @param data Buffer to receive into
 * @param len Maximum number of bytes to receive (buffer size)
 * @param cont Enable continuous RX. When set, call this once and the HAL keeps
 *             receiving frames of the same maximum length, invoking
 *             PHAL_USART_rxCallback after each.
 * @return true if reception started, false otherwise
 */
bool PHAL_USART_rxDMA(PHAL_USART_Handle_t *handle, uint8_t *data, uint32_t len, bool cont) {
    ssize_t idx = USART_PRIV_idx_from_periph(handle->periph);
    if (idx < 0) return false;
    if (usart_state[idx].handle != handle) return false;

    usart_state[idx].cont_rx = cont;
    usart_state[idx].rxfer_size = len;

    USART_PRIV_start_rx(handle->periph);

    // Channel must be disabled to set address/length; reEnable clears stale
    // flags and starts reception.
    dma_init_t *rx_dma = &usart_state[idx].rx_dma;
    PHAL_stopTxfer(rx_dma);
    PHAL_DMA_setMemAddress(rx_dma, (uint32_t)data);
    PHAL_DMA_setTxferLength(rx_dma, len);
    PHAL_reEnable(rx_dma);

    return true;
}

/**
 * @brief Check whether a DMA transmission is still in progress.
 *
 * @param handle Handle of the USART to check
 * @return true if a transmission is in flight, false otherwise
 */
bool PHAL_USART_txBusy(PHAL_USART_Handle_t *handle) {
    ssize_t idx = USART_PRIV_idx_from_periph(handle->periph);
    if (idx < 0) return false;
    return usart_state[idx].tx_busy;
}

//! On the IDLE line, finish the frame, re-arm if continuous, and notify the app.
static void PHAL_USART_HandleIRQ(ssize_t idx) {
    USART_TypeDef *periph = USART_PRIV_periph(idx);

    if (USART_PRIV_idle_active(periph)) {
        dma_init_t *rx_dma = &usart_state[idx].rx_dma;
        PHAL_stopTxfer(rx_dma);

        if (usart_state[idx].cont_rx) {
            // The transfer length has counted down to 0; reload it and re-arm.
            // reEnable clears stale channel flags so a prior TEIF can't stall re-arm.
            PHAL_DMA_setTxferLength(rx_dma, usart_state[idx].rxfer_size);
            PHAL_reEnable(rx_dma);
        } else {
            USART_PRIV_stop_rx(periph);
        }

        PHAL_USART_rxCallback(usart_state[idx].handle);
    }

    USART_PRIV_clear_status_flags(periph);
}

//! On TX DMA completion, mark the transmitter free and clear the channel flags.
static void PHAL_USART_HandleDMA(ssize_t idx) {
    if (USART_PRIV_tx_dma_complete(idx)) {
        PHAL_stopTxfer(&usart_state[idx].tx_dma);
        usart_state[idx].tx_busy = false;
    }
    USART_PRIV_clear_tx_dma_flags(idx);
}

[[gnu::weak]]
void PHAL_USART_rxCallback(PHAL_USART_Handle_t *handle) {
    (void)handle;
}

/* DMA transfer-complete interrupt handlers (TX channels) */
[[gnu::weak]]
void DMA1_Channel7_IRQHandler(void) {
    PHAL_USART_HandleDMA(USART1_IDX);
}

[[gnu::weak]]
void DMA1_Channel4_IRQHandler(void) {
    PHAL_USART_HandleDMA(USART2_IDX);
}

/// Service USART3 RX when it owns DMA1 channel 1.
void PHAL_USART_DMA1_Channel1_IRQHandler(void) {
    handleDMAxComplete(DMA1, 1, USART_DMA_RX, USART3_ACTIVE_IDX);
}

// ADC provides the strong shared vector when linked and delegates here when
// ADC1 does not own the channel. This weak vector covers USART-only builds.
[[gnu::weak]]
void DMA1_Channel1_IRQHandler(void) {
    PHAL_USART_DMA1_Channel1_IRQHandler();
}

/* USART Interrupt Handlers */
[[gnu::weak]]
void DMA1_Channel2_IRQHandler(void) {
    PHAL_USART_HandleDMA(USART3_IDX);
}

/* USART interrupt handlers (IDLE line) */
void USART1_IRQHandler(void) {
    PHAL_USART_HandleIRQ(USART1_IDX);
}

void USART2_IRQHandler(void) {
    PHAL_USART_HandleIRQ(USART2_IDX);
}

void USART3_IRQHandler(void) {
    PHAL_USART_HandleIRQ(USART3_IDX);
}
