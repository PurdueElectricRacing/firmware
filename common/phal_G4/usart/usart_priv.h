#ifndef __PHAL_G4_USART_PRIV_H__
#define __PHAL_G4_USART_PRIV_H__

#include <sys/types.h> // ssize_t

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/phal_G4.h"

// Internal slot index for each wired UART. NUM_USART auto-counts the entries
// and sizes both USART_MAP and the runtime-state array.
typedef enum {
    USART1_IDX,
    USART2_IDX,
    USART3_IDX,
    NUM_USART
} PHAL_USART_Idx_t;

// Fixed hardware wiring for one UART. Every field is dictated by the datasheet
// (RM0440 / STM32G474). This table is the single source of truth that replaces
// the old per-peripheral RCC, NVIC, and DMA-channel switch ladders.
typedef struct {
    volatile uint32_t *rcc_enable_rg; //!< RCC enable register for this UART
    USART_TypeDef *periph;            //!< peripheral instance
    DMA_TypeDef *dma;                 //!< DMA controller (TX and RX share it here)
    IRQn_Type tx_dma_irq;             //!< NVIC line for the TX DMA channel
    uint32_t tx_request;              //!< DMAMUX request id for TX
    uint32_t rx_request;              //!< DMAMUX request id for RX
    uint32_t rcc_enable_msk;          //!< enable bit within rcc_enable_rg
    IRQn_Type irq;                    //!< USART global interrupt (carries IDLE)
    uint8_t tx_channel;               //!< 1-based DMA channel for TX
    uint8_t rx_channel;               //!< 1-based DMA channel for RX
} PHAL_USART_HwMap_t;

/*
 * Private register-level operations. All USART/DMA/RCC/NVIC bit-and-register
 * business lives here so the public source (usart.c) reads as pure orchestration.
 */

//! @return slot index for periph, or -1 if it is not a supported USART.
ssize_t USART_PRIV_idx_from_periph(USART_TypeDef *periph);

//! @return the peripheral instance for a slot (used by the interrupt handlers).
USART_TypeDef *USART_PRIV_periph(ssize_t idx);

//! Enable the clock, program 8N1 + baud, and enable the IDLE + TX-DMA interrupts.
void USART_PRIV_configure(ssize_t idx, uint32_t baud_rate, uint32_t clock_rate);

//! Fill the TX and RX DMA descriptors from the hardware map.
void USART_PRIV_build_dma(ssize_t idx, dma_init_t *tx_dma, dma_init_t *rx_dma);

//! Enable the transmitter and its DMA request line (CR3.DMAT, CR1.TE).
void USART_PRIV_start_tx(USART_TypeDef *periph);

//! Enable the receiver and its DMA request line (CR1.RE, CR3.DMAR).
void USART_PRIV_start_rx(USART_TypeDef *periph);

//! Disable the receiver (CR1.RE) — used to end a one-shot reception.
void USART_PRIV_stop_rx(USART_TypeDef *periph);

//! @return true if the IDLE-line flag is set (an RX frame just completed).
bool USART_PRIV_idle_active(USART_TypeDef *periph);

//! Clear the IDLE and RX error status flags (write-1-to-clear).
void USART_PRIV_clear_status_flags(USART_TypeDef *periph);

//! @return true if the slot's TX DMA channel signalled transfer complete.
bool USART_PRIV_tx_dma_complete(ssize_t idx);

//! Clear all interrupt flags for the slot's TX DMA channel.
void USART_PRIV_clear_tx_dma_flags(ssize_t idx);

#endif // __PHAL_G4_USART_PRIV_H__
