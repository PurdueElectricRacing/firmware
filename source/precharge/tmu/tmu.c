#include "tmu.h"

void initTMU(tmu_handle_t *tmu) {
    uint8_t spi_tx_buffer[3] = {0};
    uint8_t spi_rx_buffer[3] = {0};
    while(PHAL_SPI_busy())
        ;
    spi_tx_buffer[0] = TMU_PRODID_ADDR;
    PHAL_SPI_transfer(tmu->spi, spi_tx_buffer, 3, spi_rx_buffer);
    while (PHAL_SPI_busy())
        ;
    tmu->tmu_1 = 0;
}

void readTemps(tmu_handle_t *tmu) {
    uint8_t spi_tx_buffer[11] = {0};
    uint8_t spi_rx_buffer[11] = {0};

    while(PHAL_SPI_busy())
        ;
    spi_tx_buffer[0] = (TMU_FILTERED_DATA_CMD << 2) | (0x1);
    PHAL_SPI_transfer(tmu->spi, spi_tx_buffer, 11, spi_rx_buffer);
    while (PHAL_SPI_busy())
        ;
    tmu->tmu_1 = (((float)(1.8 / 0xFFF) * (uint16_t)((spi_rx_buffer[1] << 8) | spi_rx_buffer[2])));
    tmu->tmu_2 = (((float)(1.8 / 0xFFF) * (uint16_t)((spi_rx_buffer[3] << 8) | spi_rx_buffer[4])));
    tmu->tmu_3 = (((float)(1.8 / 0xFFF) * (uint16_t)((spi_rx_buffer[5] << 8) | spi_rx_buffer[6])));
    tmu->tmu_4 = (((float)(1.8 / 0xFFF) * (uint16_t)((spi_rx_buffer[7] << 8) | spi_rx_buffer[8])));

}