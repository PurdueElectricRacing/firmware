#include "common/phal_G4/usart/usart.h"
#include "common/phal_G4/usart/usart_priv.h"

#include "common/phal_G4/dma/dma.h"

typedef struct {
    PHAL_DMA_Handle_t tx_dma;    /*!< TX DMA handle (built in init) */
    PHAL_DMA_Handle_t rx_dma;    /*!< RX DMA handle (built in init) */
    volatile uint32_t rxfer_size; /*!< configured RX length (for continuous re-arm) */
    volatile bool tx_busy;       /*!< set when a TX is in flight, cleared by the DMA ISR */
    volatile bool rx_busy;       /*!< set while a frame is in flight, cleared by the IDLE-line ISR */
    bool cont_rx;                /*!< continuous vs one-shot reception */
} PHAL_USART_state_t;

static PHAL_USART_state_t usart_state[NUM_USART];

/**
 * @brief Initialize a USART peripheral for DMA-driven communication.
 *
 * @param periph Which USART peripheral to initialize
 * @param baud_rate Desired baud rate
 * @param clock_rate Frequency (Hz) of the bus clock feeding this USART (APB1/APB2)
 * @return true on success, false if DMA init failed
 */
bool PHAL_USART_init(PHAL_USART_Idx_t periph, uint32_t baud_rate, const uint32_t clock_rate) {
    ssize_t idx = periph;

    PHAL_USART_priv_configure(idx, baud_rate, clock_rate);
    PHAL_USART_priv_buildDma(idx, &usart_state[idx].tx_dma, &usart_state[idx].rx_dma);

    if (!PHAL_DMA_init(&usart_state[idx].tx_dma) || !PHAL_DMA_init(&usart_state[idx].rx_dma)) {
        return false;
    }

    return true;
}

/**
 * @brief Start a DMA-based transmission.
 *
 * @param periph Which USART peripheral to transmit on
 * @param data Buffer to send
 * @param len Number of bytes to send
 * @return true if every DMA reconfiguration step succeeded, false otherwise
 */
bool PHAL_USART_tx(PHAL_USART_Idx_t periph, uint8_t *data, uint32_t len) {
    ssize_t idx = periph;

    usart_state[idx].tx_busy = true;

    // Re-target the TX channel at this buffer (channel must be disabled to set
    // length/address); restart clears stale flags and starts the transfer.
    // Run every step regardless of earlier ones failing - skipping restart
    // after a partial reconfiguration would leave the channel worse off, not
    // better - then report whether they all actually succeeded.
    PHAL_DMA_Handle_t *tx_dma = &usart_state[idx].tx_dma;
    bool stopped     = PHAL_DMA_stop(tx_dma);
    bool length_set  = PHAL_DMA_setLength(tx_dma, len);
    bool address_set = PHAL_DMA_setMemAddress(tx_dma, (uint32_t)data);
    bool restarted   = PHAL_DMA_restart(tx_dma);

    PHAL_USART_priv_startTx(PHAL_USART_priv_periph(idx));

    return stopped && length_set && address_set && restarted;
}

/**
 * @brief Start a DMA-based reception, completed on the IDLE line.
 *
 * @param periph Which USART peripheral to receive on
 * @param data Buffer to receive into
 * @param len Maximum number of bytes to receive (buffer size)
 * @param cont Enable continuous RX. When set, call this once and the HAL keeps
 *             receiving frames of the same maximum length, invoking
 *             PHAL_USART_rxCallback after each.
 * @return true if every DMA reconfiguration step succeeded, false otherwise
 */
bool PHAL_USART_rx(PHAL_USART_Idx_t periph, uint8_t *data, uint32_t len, bool cont) {
    ssize_t idx = periph;

    usart_state[idx].cont_rx = cont;
    usart_state[idx].rxfer_size = len;
    usart_state[idx].rx_busy = true;

    // Channel must be disabled to set address/length; restart clears stale
    // flags and starts reception. Same reasoning as txDMA above: run every
    // step, then report whether they all actually succeeded.
    PHAL_DMA_Handle_t *rx_dma = &usart_state[idx].rx_dma;
    bool stopped     = PHAL_DMA_stop(rx_dma);
    bool address_set = PHAL_DMA_setMemAddress(rx_dma, (uint32_t)data);
    bool length_set  = PHAL_DMA_setLength(rx_dma, len);
    bool restarted   = PHAL_DMA_restart(rx_dma);

    PHAL_USART_priv_startRx(PHAL_USART_priv_periph(idx));

    return stopped && address_set && length_set && restarted;
}

/**
 * @brief Check whether a DMA transmission is still in progress.
 *
 * @param periph Which USART peripheral to check
 * @return true if a transmission is in flight, false otherwise
 */
bool PHAL_USART_txBusy(PHAL_USART_Idx_t periph) {
    return usart_state[periph].tx_busy;
}

/**
 * @brief Transmit data, blocking until the transfer completes.
 *
 * @param periph Which USART peripheral to transmit on
 * @param data Buffer to send
 * @param len Number of bytes to send
 * @return true if the transfer completed, false if it failed to start
 */
bool PHAL_USART_txBlocking(PHAL_USART_Idx_t periph, uint8_t *data, uint32_t len) {
    if (!PHAL_USART_tx(periph, data, len)) return false;

    while (PHAL_USART_txBusy(periph)) {
        __asm__("nop");
    }
    
    return true;
}

/**
 * @brief Receive data, blocking until a one-shot reception completes.
 *
 * @param periph Which USART peripheral to receive on
 * @param data Buffer to receive into
 * @param len Number of bytes to receive
 * @return true if the reception completed, false if it failed to start
 */
bool PHAL_USART_rxBlocking(PHAL_USART_Idx_t periph, uint8_t *data, uint32_t len) {
    if (!PHAL_USART_rx(periph, data, len, false)) return false;

    while (usart_state[periph].rx_busy) {
        __asm__("nop");
    }

    return true;
}

/// On the IDLE line, finish the frame, re-arm if continuous, and notify the app.
static void PHAL_USART_HandleIRQ(PHAL_USART_Idx_t idx) {
    USART_TypeDef *periph = PHAL_USART_priv_periph(idx);

    if (PHAL_USART_priv_idleActive(periph)) {
        PHAL_DMA_Handle_t *rx_dma = &usart_state[idx].rx_dma;
        PHAL_DMA_stop(rx_dma);
        usart_state[idx].rx_busy = false;

        if (usart_state[idx].cont_rx) {
            // The transfer length has counted down to 0; reload it and re-arm.
            // restart clears stale channel flags so a prior TEIF can't stall re-arm.
            PHAL_DMA_setLength(rx_dma, usart_state[idx].rxfer_size);
            PHAL_DMA_restart(rx_dma);
        } else {
            PHAL_USART_priv_stopRx(periph);
        }

        PHAL_USART_rxCallback(idx);
    }

    PHAL_USART_priv_clearStatusFlags(periph);
}

/// On TX DMA completion, mark the transmitter free and clear the channel flags.
static void PHAL_USART_HandleDMA(PHAL_USART_Idx_t idx) {
    if (PHAL_USART_priv_txDmaComplete(idx)) {
        PHAL_DMA_stop(&usart_state[idx].tx_dma);
        usart_state[idx].tx_busy = false;
    }
    PHAL_USART_priv_clearTxDmaFlags(idx);
}

[[gnu::weak]]
void PHAL_USART_rxCallback(PHAL_USART_Idx_t periph) {
    (void)periph;
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
