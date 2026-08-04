#include "common/phal_G4/usart/usart_priv.h"

static const PHAL_USART_HwMap_t USART_MAP[NUM_USART] = {
    [USART1_IDX] = {
        .rcc_enable_rg  = &RCC->APB2ENR,   
        .rcc_enable_msk = RCC_APB2ENR_USART1EN,
        .periph         = USART1,
        .irq            = USART1_IRQn,     
        .tx_dma_irq     = DMA1_Channel7_IRQn,
        .tx_wiring      = &USART1_TX_DMA_WIRING,
        .rx_wiring      = &USART1_RX_DMA_WIRING,
    },
    [USART2_IDX] = {
        .rcc_enable_rg  = &RCC->APB1ENR1,  
        .rcc_enable_msk = RCC_APB1ENR1_USART2EN,
        .periph         = USART2,
        .irq            = USART2_IRQn,     
        .tx_dma_irq     = DMA1_Channel4_IRQn,
        .tx_wiring      = &USART2_TX_DMA_WIRING,
        .rx_wiring      = &USART2_RX_DMA_WIRING,
    },
    [USART3_IDX] = {
        .rcc_enable_rg  = &RCC->APB1ENR1,  
        .rcc_enable_msk = RCC_APB1ENR1_USART3EN,
        .periph         = USART3,
        .irq            = USART3_IRQn,     
        .tx_dma_irq     = DMA1_Channel2_IRQn,
        .tx_wiring      = &USART3_TX_DMA_WIRING,
        .rx_wiring      = &USART3_RX_DMA_WIRING,
    },
};

USART_TypeDef *USART_PRIV_periph(ssize_t idx) {
    return USART_MAP[idx].periph;
}

void USART_PRIV_configure(ssize_t idx, uint32_t baud_rate, uint32_t clock_rate) {
    const PHAL_USART_HwMap_t *map = &USART_MAP[idx];
    USART_TypeDef *periph = map->periph;

    // Enable the peripheral clock.
    *map->rcc_enable_rg |= map->rcc_enable_msk;

    // Reset control registers. We want: 
    // 8 data bits, no parity, 1 stop bit,
    // 16x oversampling, disabled peripheral
    periph->CR1 = 0U;
    periph->CR2 = 0U;
    periph->CR3 = 0U;

    // Per original source code
    periph->BRR = (clock_rate + (baud_rate / 2U)) / baud_rate;

    // IDLE-line interrupt signals RX frame completion.
    periph->CR1 |= USART_CR1_IDLEIE;

    // Enable USART 
    periph->CR1 |= USART_CR1_UE;

    NVIC_EnableIRQ(map->irq);
    NVIC_EnableIRQ(map->tx_dma_irq);
}

void USART_PRIV_build_dma(ssize_t idx, PHAL_DMA_Handle_t *tx_dma, PHAL_DMA_Handle_t *rx_dma) {
    const PHAL_USART_HwMap_t *map = &USART_MAP[idx];

    *tx_dma = (PHAL_DMA_Handle_t) {
        .wiring = map->tx_wiring,
        .params = {
            .priority  = DMA_PRIORITY_MEDIUM,
            .mode      = DMA_MODE_NORMAL,
            .mem_inc   = true,
            .tx_isr_en = true,
        },
    };

    *rx_dma = (PHAL_DMA_Handle_t) {
        .wiring = map->rx_wiring,
        .params = {
            .priority = DMA_PRIORITY_HIGH,
            .mode     = DMA_MODE_NORMAL,
            .mem_inc  = true,
            .tx_isr_en = false,
        },
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
    const PHAL_DMA_Wiring_t *wiring = USART_MAP[idx].tx_wiring;
    uint32_t shift = 4U * (wiring->channel_idx - 1U);
    return (wiring->periph->ISR & (DMA_ISR_TCIF1 << shift)) != 0U;
}

void USART_PRIV_clear_tx_dma_flags(ssize_t idx) {
    const PHAL_DMA_Wiring_t *wiring = USART_MAP[idx].tx_wiring;
    uint32_t shift = 4U * (wiring->channel_idx - 1U);
    // CGIF clears TCIF/HTIF/TEIF/GIF for the channel (RM0440, DMA_IFCR).
    wiring->periph->IFCR = DMA_IFCR_CGIF1 << shift;
}
