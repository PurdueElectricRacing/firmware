/**
 * @file spi.h
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

typedef enum {
    SPI_CPOL_IDLE_LOW = 0,
    SPI_CPOL_IDLE_HIGH = 1,
} SPI_CPOL;

typedef enum {
    SPI_CPHA_FIRST_EDGE = 0,
    SPI_CPHA_SECOND_EDGE = 1,
} SPI_CPHA;

typedef struct {
    uint32_t data_rate;
    uint8_t data_len;
    SPI_Mode mode;
    bool nss_sw;
    GPIO_TypeDef *nss_gpio_port;
    uint32_t nss_gpio_pin;

    SPI_CPOL cpol;
    SPI_CPHA cpha;

    PHAL_DMA_Handle_t *rx_dma; // DMA RX config (optional)
    PHAL_DMA_Handle_t *tx_dma; // DMA TX config (required for DMA path)

    volatile bool _busy;  // Busy flag
    volatile bool _error; // TX error occurred
    volatile bool _direct_mode_error;
    volatile bool _fifo_overrun;

    SPI_TypeDef *periph; // SPI peripheral base
} SPI_InitConfig_t;

void PHAL_SPI_init(SPI_InitConfig_t *handle);
void PHAL_SPI_transfer(SPI_InitConfig_t *spi,
                       const uint8_t *out_data,
                       const uint32_t data_len,
                       uint8_t *in_data);
void PHAL_SPI_transfer_blocking(SPI_InitConfig_t *spi,
                             const uint8_t *out_data,
                             uint32_t txlen,
                             uint8_t *in_data);
bool PHAL_SPI_busy(SPI_InitConfig_t *cfg);
uint8_t PHAL_SPI_writeByte(SPI_InitConfig_t *spi, uint8_t address, uint8_t writeDat);
uint8_t PHAL_SPI_readByte(SPI_InitConfig_t *spi, uint8_t address, bool skipDummy);


/**
 * @brief SPI transfer-complete callback.
 *
 * Called after a DMA-backed SPI transfer has fully completed and the handle has
 * been marked idle.
 */
extern void PHAL_SPI_txCallback(SPI_InitConfig_t *spi);

#endif /* _PHAL_G4_SPI_H */
