/**
 * @file dma.h
 * @author Dawson Moore (moore800@purdue.edu)
 * @brief Basic DMA Peripheral HAL library for setting up DMA transfers
 * @version 0.1
 * @date 2022-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _DMA_H_
#define _DMA_H_

#ifdef STM32L496xx
#include "stm32l496xx.h"
#elif STM32L471xx
#include "stm32l471xx.h"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include <stdbool.h>

/** Top-level DMA configuration */
typedef struct {
    uint32_t    periph_addr; //!< Address of the peripheral location (or other memory location in mem-to-mem) to transfer from/to
    uint32_t    mem_addr;    //!< Address of the memory location to transfer from/to
    uint16_t    tx_size;     //!< Number of transfers to complete in sizes set by mem / periph size

    bool        increment;
    bool        circular;    //!< Continuously transfer
    uint8_t     dir;         //!< If set to 0, transfers from periph_addr to mem_addr. If 1, opposite direction.
    bool        mem_inc;     //!< Increment mem_addr after each transfer
    bool        periph_inc;  //!< Increment periph_addr after each transfer
    bool        mem_to_mem;  //!< If both locations are memory locations
    uint8_t     priority;    //!< Transfer priority
    uint8_t     mem_size;    //!< Size to read from mem_addr    (00 = 8 bits, 01 = 16 bits, 10 = 32 bits)
    uint8_t     periph_size; //!< Size to read form periph_addr (00 = 8 bits, 01 = 16 bits, 10 = 32 bits)
    bool        tx_isr_en;   //!< Enable the TX ISR
    uint8_t     dma_chan_request; //!< Table 44 of Family Reference  */
    uint8_t     channel_idx; //!< DMA Channel (Table 44 of Family Reference)

    DMA_TypeDef* periph;
    DMA_Channel_TypeDef* channel;
    DMA_Request_TypeDef* request;
} dma_init_t;

/**
 * @brief Initialize DMA peripheral to set m2m, p2p, or p2m with set size
 *        and length of txfer
 *
 * @param init -> Address of initialization structure
 * @return true -> Successful init (no clashing params)
 * @return false -> Init not complete (parameters clash)
 */
bool PHAL_initDMA(dma_init_t* init);

/**
 * @brief Start txfer after sucessful DMA peripheral initialization
 *
 * @param init -> Address of initialization structure
 */
void PHAL_startTxfer(dma_init_t* init);

/**
 * @brief Stop txfer
 *
 * @param init -> Address of initialization structure
 */
void PHAL_stopTxfer(dma_init_t* init);

/**
 * @brief Re-enable DMA txfer after error ISR fires
 *
 * @param init -> Address of initialization structure
 */
void PHAL_reEnable(dma_init_t* init);

/**
 * @brief Set memory address for DMA transfer. In Mem to Mem this acts as the source address
 *
 * @param init -> Address of initialization structure
 */
void PHAL_DMA_setMemAddress(dma_init_t* init, const uint32_t address);

/**
 * @brief Set transfer length for DMA transaction
 *
 * @param init -> Address of initialization structure
 */
void PHAL_DMA_setTxferLength(dma_init_t* init, const uint32_t length);

#endif