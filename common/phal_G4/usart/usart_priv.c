#include "common/phal_G4/usart/usart_priv.h"

// Single source of truth for per-UART hardware wiring.
static const PHAL_USART_HwMap_t USART_MAP[NUM_USART] = {
    [USART1_IDX] = {
        .rcc_enable_rg  = &RCC->APB2ENR,   .rcc_enable_msk = RCC_APB2ENR_USART1EN,
        .periph         = USART1,          .dma            = DMA1,
        .irq            = USART1_IRQn,     .tx_dma_irq     = DMA1_Channel7_IRQn,
        .tx_channel     = 7,               .tx_request     = DMA_REQUEST_USART1_TX,
        .rx_channel     = 5,               .rx_request     = DMA_REQUEST_USART1_RX,
    },
    [USART2_IDX] = {
        .rcc_enable_rg  = &RCC->APB1ENR1,  .rcc_enable_msk = RCC_APB1ENR1_USART2EN,
        .periph         = USART2,          .dma            = DMA1,
        .irq            = USART2_IRQn,     .tx_dma_irq     = DMA1_Channel4_IRQn,
        .tx_channel     = 4,               .tx_request     = DMA_REQUEST_USART2_TX,
        .rx_channel     = 3,               .rx_request     = DMA_REQUEST_USART2_RX,
    },
    [USART3_IDX] = {
        .rcc_enable_rg  = &RCC->APB1ENR1,  .rcc_enable_msk = RCC_APB1ENR1_USART3EN,
        .periph         = USART3,          .dma            = DMA1,
        .irq            = USART3_IRQn,     .tx_dma_irq     = DMA1_Channel2_IRQn,
        .tx_channel     = 2,               .tx_request     = DMA_REQUEST_USART3_TX,
        .rx_channel     = 1,               .rx_request     = DMA_REQUEST_USART3_RX,
    },
};

ssize_t USART_PRIV_idx_from_periph(USART_TypeDef *periph) {
    for (uint8_t i = 0; i < NUM_USART; i++) {
        if (USART_MAP[i].periph == periph) {
            return i;
        }
    }
    return -1;
}

USART_TypeDef *USART_PRIV_periph(ssize_t idx) {
    return USART_MAP[idx].periph;
}

void USART_PRIV_configure(ssize_t idx, uint32_t baud_rate, uint32_t clock_rate) {
    const PHAL_USART_HwMap_t *map = &USART_MAP[idx];
    USART_TypeDef *periph = map->periph;

    // Enable the peripheral clock.
    *map->rcc_enable_rg |= map->rcc_enable_msk;

    // Reset control registers. The all-zero defaults give the desired frame
    // format: 8 data bits, no parity, 1 stop bit, 16x oversampling.
    periph->CR1 = 0U;
    periph->CR2 = 0U;
    periph->CR3 = 0U;

    periph->BRR = (clock_rate + (baud_rate / 2U)) / baud_rate;

    // IDLE-line interrupt signals RX frame completion.
    periph->CR1 |= USART_CR1_IDLEIE;
    periph->CR1 |= USART_CR1_UE;

    // Route interrupts to the CPU: USART IDLE (RX) and TX DMA complete.
    NVIC_EnableIRQ(map->irq);
    NVIC_EnableIRQ(map->tx_dma_irq);
}

void USART_PRIV_build_dma(ssize_t idx, dma_init_t *tx_dma, dma_init_t *rx_dma) {
    const PHAL_USART_HwMap_t *map = &USART_MAP[idx];

    *tx_dma = (dma_init_t) {
        .periph_addr = (uint32_t)&map->periph->TDR,
        .periph      = map->dma,
        .channel_idx = map->tx_channel,
        .mux_request = map->tx_request,
        .mem_size    = DMA_SIZE_8BIT,
        .periph_size = DMA_SIZE_8BIT,
        .mem_inc     = true,
        .tx_isr_en   = true, // TX completion is signalled by the DMA interrupt
        .dir         = 1,
        .priority    = 1,
    };

    *rx_dma = (dma_init_t) {
        .periph_addr = (uint32_t)&map->periph->RDR,
        .periph      = map->dma,
        .channel_idx = map->rx_channel,
        .mux_request = map->rx_request,
        .mem_size    = DMA_SIZE_8BIT,
        .periph_size = DMA_SIZE_8BIT,
        .mem_inc     = true,
        // no tx_isr_en: RX completion comes from the USART IDLE line, not DMA
        .dir         = 0,
        .priority    = 2,
    };
}

void USART_PRIV_start_tx(USART_TypeDef *periph) {
    periph->CR3 |= USART_CR3_DMAT;
    periph->CR1 |= USART_CR1_TE;
}

void USART_PRIV_start_rx(USART_TypeDef *periph) {
    periph->CR1 |= USART_CR1_RE;
    periph->CR3 |= USART_CR3_DMAR;
}

void USART_PRIV_stop_rx(USART_TypeDef *periph) {
    periph->CR1 &= ~USART_CR1_RE;
}

bool USART_PRIV_idle_active(USART_TypeDef *periph) {
    return (periph->ISR & USART_ISR_IDLE) != 0U;
}

void USART_PRIV_clear_status_flags(USART_TypeDef *periph) {
    // ICR is write-1-to-clear; clearing a flag that is not set is harmless.
    periph->ICR = USART_ICR_IDLECF | USART_ICR_ORECF | USART_ICR_NECF
                | USART_ICR_FECF | USART_ICR_PECF;
}

bool USART_PRIV_tx_dma_complete(ssize_t idx) {
    const PHAL_USART_HwMap_t *map = &USART_MAP[idx];
    uint32_t shift = 4U * (map->tx_channel - 1U);
    return (map->dma->ISR & (DMA_ISR_TCIF1 << shift)) != 0U;
}

void USART_PRIV_clear_tx_dma_flags(ssize_t idx) {
    const PHAL_USART_HwMap_t *map = &USART_MAP[idx];
    uint32_t shift = 4U * (map->tx_channel - 1U);
    // CGIF clears TCIF/HTIF/TEIF/GIF for the channel (RM0440, DMA_IFCR).
    map->dma->IFCR = DMA_IFCR_CGIF1 << shift;
}
