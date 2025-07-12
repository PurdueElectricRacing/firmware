/**
 * @file dma.h
 * @author Chris McGalliard - Port of L4 HAL by Dawson Moore (moore800@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2023-08-19
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __PHAL_G4_DMA_H__
#define __PHAL_G4_DMA_H__

#include "common/phal_G4/phal_G4.h"
typedef enum {
    DMA_SIZE_8BIT  = 0,
    DMA_SIZE_16BIT = 1,
    DMA_SIZE_32BIT = 2
} dma_size_t;

// Mux requests (TODO support all)
#define DMA_REQUEST_ADC1               5U

#define DMA_REQUEST_SPI1_RX           10U
#define DMA_REQUEST_SPI1_TX           11U
#define DMA_REQUEST_SPI2_RX           12U
#define DMA_REQUEST_SPI2_TX           13U
#define DMA_REQUEST_SPI3_RX           14U
#define DMA_REQUEST_SPI3_TX           15U

#define DMA_REQUEST_USART1_RX         24U
#define DMA_REQUEST_USART1_TX         25U
#define DMA_REQUEST_USART2_RX         26U
#define DMA_REQUEST_USART2_TX         27U
#define DMA_REQUEST_USART3_RX         28U
#define DMA_REQUEST_USART3_TX         29U

typedef struct {
    uint32_t    periph_addr;
    uint32_t    mem_addr;
    uint16_t    tx_size;
    uint8_t     mem_size;

    bool        increment;
    bool        circular;
    uint8_t     dir;
    bool        mem_inc;
    bool        periph_inc;
    bool        mem_to_mem;
    uint8_t     priority;
    uint8_t     periph_size;
    bool        tx_isr_en;
    uint8_t     dma_chan_request;
    uint8_t     channel_idx;
    uint8_t     mux_request;

    DMA_TypeDef* periph;
    DMA_Channel_TypeDef* channel; // Example DMA1_Stream0 or DMA2_Stream7
} dma_init_t;

// TODO ADC3 config (ADC2 doesn't support DMA)
#define ADC1_DMA_CONT_CONFIG(mem_addr_, tx_size_, priority_)       \
    {.periph_addr=(uint32_t)&(ADC1->DR), .mem_addr=mem_addr_,      \
     .tx_size=tx_size_, .increment=true, .circular=true, .dir=0b0, \
     .mem_inc=true, .periph_inc=false, .mem_to_mem=false,          \
     .priority=priority_,                                          \
     .mem_size=DMA_SIZE_16BIT, .periph_size=DMA_SIZE_16BIT,        \
     .tx_isr_en=false, .dma_chan_request=0b0000, .channel_idx=1,   \
     .mux_request=DMA_REQUEST_ADC1, .periph=DMA1, .channel=DMA1_Channel1}

/*
 * @brief Initialize DMA peripheral to set m2m, p2p, or p2m with set size
 *        and length of txfer
 *
 * @param dma -> Address of initialization structure
 * @return true -> Successful init (no clashing params)
 * @return false -> Init not complete (parameters clash)
 */
bool PHAL_initDMA(dma_init_t* dma);

/*
 * @brief Start txfer after sucessful DMA peripheral initialization
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_startTxfer(dma_init_t* dma);

/*
 * @brief Stop txfer
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_stopTxfer(dma_init_t* dma);

/*
 * @brief Re-enable DMA txfer after error ISR fires
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_reEnable(dma_init_t* dma);

/*
 * @brief Set memory address for DMA transfer. In Mem to Mem this acts as the source address
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_setMemAddress(dma_init_t* dma, const uint32_t address);

/*
 * @brief Set transfer length for DMA transaction
 *
 * @param dma -> Address of initialization structure
 */
void PHAL_DMA_setTxferLength(dma_init_t* dma, const uint32_t length);

#endif // __PHAL_G4_DMA_H__
