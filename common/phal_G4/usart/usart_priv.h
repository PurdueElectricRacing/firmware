#ifndef __PHAL_G4_USART_PRIV_H__
#define __PHAL_G4_USART_PRIV_H__

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/phal_G4.h"

typedef enum {
    USART1_IDX,
    USART2_IDX,
    USART3_IDX,
    NUM_USART
} PHAL_USART_Idx_t;

typedef struct {
    volatile uint32_t*  rcc_enable_rg;
    USART_TypeDef*      periph;
    DMA_TypeDef*        dma;
    IRQn_Type           tx_dma_irq;
    uint32_t            tx_request;
    uint32_t            rx_request;
    uint32_t            rcc_enable_msk;
    IRQn_Type           irq;
    uint8_t             tx_channel;
    uint8_t             rx_channel;
} PHAL_USART_HwMap_t;

#endif