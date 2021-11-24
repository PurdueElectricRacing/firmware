#include "spi_flash.h"

#define SPI_FLASH_RDSR_CMD (0x05)
#define SPI_FLASH_WEN_CMD  (0x06)

bool spiFlashCheckSR(uint32_t status_register_mask, uint32_t status_register_flags)
{
    PHAL_qspiSetFunctionMode(QUADSPI_INDIRECT_READ_MODE);
    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_SKIP_SECTION);
    PHAL_qspiSetDataWidth(QUADSPI_SINGLE_LINE);

    uint32_t spi_sr = 0xFFFF;
    PHAL_qspiRead(SPI_FLASH_RDSR_CMD, 0x0, &spi_sr, 1);

    return (spi_sr & status_register_mask) == status_register_flags;
}

void spiFlashWriteEnable()
{
    PHAL_qspiSetFunctionMode(QUADSPI_INDIRECT_WRITE_MODE);
    PHAL_qspiSetInstructionWidth(QUADSPI_SINGLE_LINE);
    PHAL_qspiSetAddressWidth(QUADSPI_SKIP_SECTION);
    PHAL_qspiSetDataWidth(QUADSPI_SKIP_SECTION);

    uint32_t dummy;
    PHAL_qspiWrite(SPI_FLASH_RDSR_CMD, 0x0, &dummy, 0);
}
