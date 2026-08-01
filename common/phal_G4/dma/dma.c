/**
 * @file dma.c
 * @brief G4 DMA Peripheral public API implementation
 * @author Shriya Balu (balu@purdue.edu)
 * 
 */

#include "common/phal_G4/dma/dma.h"
#include "common/phal_G4/dma/dma_priv.h"

bool PHAL_initDMA(dma_init_t *dma) {
    PHAL_DMA_priv_enableClock(dma);

    PHAL_DMA_priv_setChannel(dma->periph, &dma->channel, dma->channel_idx);

    /* DMA Channel configuration procedure (see RM0440 12.4.5 DMA channels section) */

    // Ensure the stream is disabled before attempting to configure the DMA control registers
    PHAL_DMA_priv_disableStream(dma->channel);
    // Clear any ISR status flags that may have been set previously for the target channel
    PHAL_DMA_priv_clearFlags(dma->periph, dma->channel_idx);
    // 1. Set the peripheral register address in the DMA_CPARx register
    PHAL_DMA_priv_setPeriphAddress(dma);
    // 2. Set the memory address in the DMA_CMARx register. 
    PHAL_DMA_setMemAddress(dma, dma->mem_addr);
    // 3. Configure the total number of data to transfer in the DMA_CNDTRx register.
    PHAL_DMA_setTxferLength(dma, dma->tx_size);
    // 4. Configure parameters in the DMA_CCRx register:
    PHAL_DMA_priv_configParams(dma);

    /* DMA Mux */
    /// DMA Mux Channel configuration procedure (see RM0440 13.4.3 DMAMUX channels section)
    PHAL_DMA_priv_configMUX(dma);

    // 5. Activate the channel by setting the EN bit in the DMA_CCRx register
    PHAL_DMA_priv_enableStream(dma->channel);
    return true;
}

void PHAL_DMA_startTxfer(dma_init_t *dma) {
    PHAL_DMA_priv_enableStream(dma->channel);
}

void PHAL_DMA_stopTxfer(dma_init_t *dma) {
    PHAL_DMA_priv_disableStream(dma->channel);
}

void PHAL_DMA_reEnable(dma_init_t *dma) {
    PHAL_DMA_priv_clearFlags(dma->periph, dma->channel_idx);
    PHAL_DMA_priv_enableStream(dma->channel);
}

void PHAL_DMA_setMemAddress(dma_init_t *dma, const uint32_t address) {
    PHAL_DMA_priv_writeMemAddress(dma, address);
}

void PHAL_DMA_setTxferLength(dma_init_t *dma, const uint32_t length) {
    PHAL_DMA_priv_writeTxferLength(dma, length);
}
