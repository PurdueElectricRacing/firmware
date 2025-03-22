#include "m95p32.h"

static inline void M95_selectDevice(M95_t* M95)
{
    M95->spi->periph->CR1 |= SPI_CR1_SPE;
    PHAL_writeGPIO(M95->spi->nss_gpio_port, M95->spi->nss_gpio_pin, 0);
}

static inline void M95_deselectDevice(M95_t* M95)
{
    M95->spi->periph->CR1 &= ~SPI_CR1_SPE;
    PHAL_writeGPIO(M95->spi->nss_gpio_port, M95->spi->nss_gpio_pin, 1);
}

static bool M95_checkManufacturerCode(M95_t* M95)
{
    uint8_t manufacturer_code = 0;
    const uint8_t expected_code = 0x20;

    M95_readAddress(M95, M95_RDID, 0x000000, &manufacturer_code, 1);

    return (manufacturer_code == expected_code);
}

bool M95_init(M95_t* M95)
{
    M95->is_initialized = false;

    if (false == M95_checkManufacturerCode(M95))
    {
        return false;
    }

    M95->is_initialized = true;
    return true;
}

static void PHAL_SPI_transfer_noDMA_M95(SPI_InitConfig_t *spi, const uint8_t *tx_data, uint32_t txlen, uint32_t rxlen, uint8_t *rx_data)
{
    for (uint32_t i = 0; i < txlen; i++)
    {
        spi->periph->DR = tx_data[i];
        while (!(spi->periph->SR & SPI_SR_TXE));
    }

    while (spi->periph->SR & SPI_SR_BSY);

    (void)spi->periph->DR;
    (void)spi->periph->SR;

    for (uint32_t i = 0; i < rxlen; i++)
    {
        spi->periph->DR = 0;

        while (!(spi->periph->SR & SPI_SR_RXNE));
        rx_data[i] = (uint8_t)(spi->periph->DR);
    }
}

void M95_readAddress(M95_t* M95, uint8_t command, uint32_t addr, uint8_t* rx_data, uint32_t rx_len)
{
    if (addr > 0xFFFFFF)
    {
        return;
    }

    uint8_t tx_cmd[4];
    tx_cmd[0] = command;
    tx_cmd[1] = (addr >> 16) & 0xFF;
    tx_cmd[2] = (addr >> 8) & 0xFF;
    tx_cmd[3] = addr & 0xFF;

    M95_selectDevice(M95);
    PHAL_SPI_transfer_noDMA_M95(M95->spi, tx_cmd, sizeof(tx_cmd), rx_len, rx_data);
    M95_deselectDevice(M95);
}

void M95_readBytes(M95_t* M95, uint8_t command, uint8_t* rx_data, uint32_t rx_len)
{
    M95_selectDevice(M95);
    PHAL_SPI_transfer_noDMA_M95(M95->spi, &command, sizeof(command), rx_len, rx_data);
    M95_deselectDevice(M95);
}

void busy_wait_cycles(uint32_t cycles)
{
    for (volatile uint32_t i = 0; i < cycles; i++);
}

bool M95_reset(M95_t* M95)
{
    uint8_t enable_reset_cmd = M95_RSTEN;
    uint8_t reset_cmd = M95_RESET;

    // Enable Reset
    M95_selectDevice(M95);
    PHAL_SPI_transfer_noDMA_M95(M95->spi, &enable_reset_cmd, 1, 0, NULL);
    M95_deselectDevice(M95);

    // Reset
    M95_selectDevice(M95);
    PHAL_SPI_transfer_noDMA_M95(M95->spi, &reset_cmd, 1, 0, NULL);
    M95_deselectDevice(M95);

    // Check safety byte
    // Todo since if not write in progress it's like 30us,
    // probably better to just hard wait for that instead of busy wait
    uint8_t safetyStatus = 0xFF;
    uint8_t attempts = 0;
    while ((safetyStatus & 0b01000000) && (attempts++ < 11))
    {
        busy_wait_cycles(480000);
        safetyStatus = M95_getSafetyRegister(M95);
    }
    bool result = ((safetyStatus & 0b01000000) == 0);

    return result;
}

uint8_t M95_getSafetyRegister(M95_t* M95)
{
    uint8_t reg_data[2] = {0xFF, 0xFF};
    M95_readBytes(M95, M95_RDCR, reg_data, 2);
    return (reg_data[1]);
}

uint8_t M95_getConfigurationRegister(M95_t* M95)
{
    uint8_t reg_data[2] = {0xFF, 0xFF};
    M95_readBytes(M95, M95_RDCR, reg_data, 2);
    return (reg_data[0]);
}

// Modify EEPROM data
// Wait until any pending WIP are finished
// The write enable command must precede any instructions for modifying data
// The chip select line must go high after each instruction
