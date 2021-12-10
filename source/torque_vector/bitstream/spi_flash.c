#include "spi_flash.h"

#define SPI_FLASH_RDSR_CMD          (0x05)
#define SPI_FLASH_WEN_CMD           (0x06)
#define SPI_FLASH_CHIP_ERASE_CMD    (0x60)
#define SPI_FLASH_SECT_ERASE_CMD    (0x20)
#define SPI_FLASH_PP_CMD            (0x02)
#define SPI_FLASH_4PP_CMD           (0x38)
#define SPI_FLASH_READ_CMD          (0x03) // Read data bytes
#define SPI_FLASH_RDID_CMD          (0x9F) // Read IDentification


void _spi_flash_send_cmd(uint8_t cmd)
{
    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_SKIP_SECTION);
    PHAL_qspiSetDataWidth(QUADSPI_SKIP_SECTION);

    uint8_t dummy[1];
    PHAL_qspiWrite(cmd, 0x0, dummy, 0);
}

bool spiFlashCheckSR(uint8_t status_register_mask, uint8_t status_register_flags)
{
    return (spiFlashReadSR() & status_register_mask) == status_register_flags;
}

uint8_t spiFlashReadSR()
{
    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_SKIP_SECTION);
    PHAL_qspiSetDataWidth(QUADSPI_SINGLE_LINE);

    uint8_t spi_sr[4] = {0};
    PHAL_qspiRead(SPI_FLASH_RDSR_CMD, 0x0, spi_sr, 2);

    return spi_sr[0];
}

uint32_t spiFlashReadID()
{
    PHAL_qspiSetAddressWidth(QUADSPI_SKIP_SECTION);
    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetDataWidth(QUADSPI_SINGLE_LINE);
    
    uint8_t data[4] = {0};
    PHAL_qspiRead(SPI_FLASH_RDID_CMD, 0x0, data, 2);
    return ((uint32_t)data[0]) | (((uint32_t)data[1]) << 8);
}

void spiFlashWriteEnable()
{
    _spi_flash_send_cmd(SPI_FLASH_WEN_CMD);
}


void spiFlashChipErase()
{
    _spi_flash_send_cmd(SPI_FLASH_CHIP_ERASE_CMD);
}

bool spiFlashSectorErase(uint32_t sector)
{
    uint8_t sr = spiFlashReadSR();
    if (!spiFlashCheckSR(0b1, 0b0) || sr == 0xff)
        return false; // Write in progress
    spiFlashWriteEnable();
    int i = 0;
    for (; i < 10; i ++)
        if (spiFlashCheckSR(0b11, 0b10))
            break;
    if (i == 10) return false;

    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressSize(QUADSPI_24_BIT);
    PHAL_qspiSetDataWidth(QUADSPI_SKIP_SECTION);

    uint8_t dummy[1];
    return PHAL_qspiWrite(SPI_FLASH_SECT_ERASE_CMD, sector * 0x1000, dummy, 0);
}

bool spiFlashProgramBytes(uint32_t address, uint32_t length, uint8_t* data)
{
    if (address / 256 != (address+length) / 256) // Data must be contained in the same 256byte page
        return false;
    uint8_t sr = spiFlashReadSR();
    if (!spiFlashCheckSR(0b1, 0b0) || sr == 0xff)
        return false; // Write in progress
    spiFlashWriteEnable();

    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_QUAD_LINE);
    PHAL_qspiSetAddressSize(QUADSPI_24_BIT);
    PHAL_qspiSetDataWidth(QUADSPI_QUAD_LINE);

    return PHAL_qspiWrite(SPI_FLASH_4PP_CMD, address, data, length);
}

bool spiFlashReadBytes(uint32_t address, uint32_t length, uint8_t* data)
{
    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressSize(QUADSPI_24_BIT);
    PHAL_qspiSetDataWidth(QUADSPI_SINGLE_LINE);

    return PHAL_qspiRead(SPI_FLASH_READ_CMD, address, data, length);
}