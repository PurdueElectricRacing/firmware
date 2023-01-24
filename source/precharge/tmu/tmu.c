#include "tmu.h"

bool initTMU(tmu_handle_t *tmu) {
    uint8_t spi_tx_buffer[3] = {0};
    uint8_t spi_rx_buffer[3] = {0};
    while(PHAL_SPI_busy(tmu->spi))
        ;
    spi_tx_buffer[0] = TMU_PRODID_ADDR;
    PHAL_SPI_transfer(tmu->spi, spi_tx_buffer, 3, spi_rx_buffer);
    while (PHAL_SPI_busy(tmu->spi))
        ;
    if (spi_rx_buffer[0] | spi_rx_buffer[1] == 0 && spi_rx_buffer[3] == 129) {
        tmu->tmu_1 = 0;
        tmu->tmu_2 = 0;
        tmu->tmu_3 = 0;
        tmu->tmu_4 = 0;
        return true;
    }
    return false;
}

void readTemps(tmu_handle_t *tmu) {
    uint8_t spi_tx_buffer[11] = {0};
    uint8_t spi_rx_buffer[11] = {0};

    while(PHAL_SPI_busy(tmu->spi))
        ;
    spi_tx_buffer[0] = (TMU_FILTERED_DATA_CMD << 2) | (0x1);
    PHAL_SPI_transfer(tmu->spi, spi_tx_buffer, 11, spi_rx_buffer);
    while (PHAL_SPI_busy(tmu->spi))
        ;
    tmu->tmu_1 = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[1] << 8) | spi_rx_buffer[2])));
    tmu->tmu_2 = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[3] << 8) | spi_rx_buffer[4])));
    tmu->tmu_3 = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[5] << 8) | spi_rx_buffer[6])));
    tmu->tmu_4 = (((TMU_VREF / TMU_ADDR_SIZE) * (uint16_t)((spi_rx_buffer[7] << 8) | spi_rx_buffer[8])));

}