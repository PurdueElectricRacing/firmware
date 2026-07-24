/**
 * @file spi.c
 * @author Ronak Jain (jain717@purdue.edu)
 * @author Shriya Balu (balu@purdue.edu)
 * @brief G4 SPI
 * @version 0.1
 */

#ifndef _PHAL_G4_SPI_H
#define _PHAL_G4_SPI_H

#include <stddef.h>

#include "common/phal_G4/dma/dma.h"

typedef enum {
    SPI_MODE_MASTER = 0,
    SPI_MODE_SLAVE  = 1,
} SPI_Mode;

typedef struct {
    uint32_t data_rate;
    uint8_t data_len;
    SPI_Mode mode;
    bool nss_sw;
    GPIO_TypeDef *nss_gpio_port;
    uint32_t nss_gpio_pin;

    uint8_t cpol;
    uint8_t cpha;

    PHAL_DMA_Handle_t *rx_dma; // DMA RX config (optional)
    PHAL_DMA_Handle_t *tx_dma; // DMA TX config (required for DMA path)

    volatile bool _busy;  // Busy flag
    volatile bool _error; // TX error occurred
    volatile bool _direct_mode_error;
    volatile bool _fifo_overrun;

    SPI_TypeDef *periph; // SPI peripheral base
} SPI_InitConfig_t;

bool PHAL_SPI_init(SPI_InitConfig_t *handle);
bool PHAL_SPI_transfer(SPI_InitConfig_t *spi,
                       const uint8_t *out_data,
                       const uint32_t data_len,
                       uint8_t *in_data);
bool PHAL_SPI_transfer_noDMA(SPI_InitConfig_t *spi,
                             const uint8_t *out_data,
                             uint32_t txlen,
                             uint32_t rxlen,
                             uint8_t *in_data);
bool PHAL_SPI_busy(SPI_InitConfig_t *cfg);
uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat);
uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy);
void PHAL_SPI_ForceReset(SPI_InitConfig_t *spi);

/**
 * @brief SPI transfer-complete callback.
 *
 * Called after a DMA-backed SPI transfer has fully completed and the handle has
 * been marked idle.
 */
extern void PHAL_SPI_txCallback(SPI_InitConfig_t *spi);

#endif /* _PHAL_G4_SPI_H */
