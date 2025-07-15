#include "quadspi.h"

static inline void qspiWaitFree() {
    // Wait for QUADSPI to not be busy
    while (!QUADSPI->SR & QUADSPI_SR_BUSY_Msk)
        ;
}

bool PHAL_qspiInit() {
    // Enable QSPI clock
    RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN;

    QUADSPI_Config_t default_config = {
        .mode = QUADSPI_INDIRECT_WRITE_MODE,

        .instruction_lines = QUADSPI_QUAD_LINE,
        .single_instruction = true,

        .address_lines = QUADSPI_QUAD_LINE,
        .address_size = QUADSPI_24_BIT,

        .alternate_lines = QUADSPI_SKIP_SECTION, // Do not use alternate bytes section
        .alternate_size = QUADSPI_8_BIT,

        .data_lines = QUADSPI_QUAD_LINE,

        .dummy_cycles = 2,
        .fifo_threshold = 1 // Begin DMA transfer when there is at least 1 available spot
    };

    // Default QUADSPI configuration
    PHAL_qspiConfigure(&default_config);
}

bool PHAL_qspiConfigure(QUADSPI_Config_t* config) {
    // Wait for QUADSPI to not be busy
    qspiWaitFree();

    // Configure FIFO threshold value, must be done before DMA configuration
    config->fifo_threshold = config->fifo_threshold <= 0 ? 1 : config->fifo_threshold; // Minimum value of 1
    QUADSPI->CR &= ~(QUADSPI_CR_FTHRES_Msk);
    QUADSPI->CR |= ((config->fifo_threshold - 1) << QUADSPI_CR_FTHRES_Pos) & QUADSPI_CR_FTHRES_Msk;

    // Function mode
    QUADSPI->CCR &= ~QUADSPI_CCR_FMODE_Msk;
    QUADSPI->CCR |= (config->mode << QUADSPI_CCR_FMODE_Pos) & QUADSPI_CCR_FMODE_Msk;

    // Data transfer width
    QUADSPI->CCR &= ~QUADSPI_CCR_DMODE_Msk;
    QUADSPI->CCR |= (config->data_lines << QUADSPI_CCR_DMODE_Pos) & QUADSPI_CCR_DMODE_Msk;

    // Address transfer width
    QUADSPI->CCR &= ~QUADSPI_CCR_ADMODE_Msk;
    QUADSPI->CCR |= (config->address_lines << QUADSPI_CCR_ADMODE_Pos) & QUADSPI_CCR_ADMODE_Msk;

    // Address transfer size
    QUADSPI->CCR &= ~QUADSPI_CCR_ADSIZE_Msk;
    QUADSPI->CCR |= (config->address_size << QUADSPI_CCR_ADSIZE_Pos) & QUADSPI_CCR_ADSIZE_Msk;

    // Transfer dummy cycles
    QUADSPI->CCR &= ~QUADSPI_CCR_DCYC_Msk;
    QUADSPI->CCR |= (config->dummy_cycles << QUADSPI_CCR_DCYC_Pos) & QUADSPI_CCR_DCYC_Msk;

    // Single instruction mode
    QUADSPI->CCR &= ~QUADSPI_CCR_SIOO_Msk;
    QUADSPI->CCR |= (config->single_instruction << QUADSPI_CCR_SIOO_Pos) & QUADSPI_CCR_SIOO_Msk;

    return true;
}

bool PHAL_qspiTrasnfer(uint8_t instruction, uint32_t address, uint8_t* data, uint32_t length) {
    qspiWaitFree();
    QUADSPI->CR &= ~(QUADSPI_CR_DMAEN_Msk | QUADSPI_CR_EN);

    // Set instruction
    QUADSPI->CCR &= ~QUADSPI_CCR_INSTRUCTION_Msk;
    QUADSPI->CCR |= (instruction << QUADSPI_CCR_INSTRUCTION_Pos) & QUADSPI_CCR_INSTRUCTION_Msk;

    // Set Address
    QUADSPI->AR = address;

    if (length > 0) {
        // *** Populate up FIFO ***
        // Ensure FIFO is empty
        if (QUADSPI->SR & QUADSPI_SR_FLEVEL)
            QUADSPI->CR |= QUADSPI_CR_ABORT;

        length = length > 16 ? 16 : length; // Maximum size without DMA is 16 bytes at a time.
        QUADSPI->DLR = length;
        for (uint8_t i = 0; i < length; i++)
            QUADSPI->DR = data[i]; // Fill up fifo
    }

    // *** Begin Transfer ***
    QUADSPI->CR |= QUADSPI_CR_EN;
}

bool PHAL_qspiTrasnfer_DMA(uint8_t instruction, uint32_t address, uint8_t* data, uint32_t length) {
    // DMA transfer enabled on FIFO Threshold flag
    QUADSPI->CR |= QUADSPI_CR_DMAEN;
}
