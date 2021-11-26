#include "quadspi.h"

static inline void qspiWaitFree()
{
    // Wait for QUADSPI to not be busy
    while (!QUADSPI->SR & QUADSPI_SR_BUSY_Msk)
        ;
}

bool PHAL_qspiInit()
{
    // Enable QSPI clock
    RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN;

    QUADSPI_Config_t default_config = {
        .mode=QUADSPI_INDIRECT_WRITE_MODE,

        .instruction_lines=QUADSPI_QUAD_LINE,
        .single_instruction=true,

        .address_lines=QUADSPI_QUAD_LINE,
        .address_size=QUADSPI_24_BIT,

        .alternate_lines=QUADSPI_SKIP_SECTION, // Do not use alternate bytes section
        .alternate_size=QUADSPI_8_BIT,

        .data_lines=QUADSPI_QUAD_LINE,

        .dummy_cycles=2,
        .fifo_threshold=1 // Begin DMA transfer when there is at least 1 available spot
    };

    // Default QUADSPI configuration
    // PHAL_qspiConfigure(&default_config);

    return true;
}

bool PHAL_qspiConfigure(QUADSPI_Config_t* config)
{
    // Wait for QUADSPI to not be busy
    qspiWaitFree();

    // Configure FIFO threshold value, must be done before DMA configuration
    config->fifo_threshold = config->fifo_threshold <= 0 ? 1 : config->fifo_threshold; // Minimum value of 1
    QUADSPI->CR &= ~(QUADSPI_CR_FTHRES_Msk);
    QUADSPI->CR |=  ((config->fifo_threshold - 1) << QUADSPI_CR_FTHRES_Pos) &QUADSPI_CR_FTHRES_Msk;

    // Function mode
    PHAL_qspiSetFunctionMode(config->mode);
    PHAL_qspiSetDataWidth(config->data_lines);
    PHAL_qspiSetAddressWidth(config->address_lines);
    PHAL_qspiSetAddressSize(config->address_size);
    PHAL_qspiSetInstructionWidth(config->instruction_lines);

    uint32_t temp_ccr = QUADSPI->CCR;

    // Transfer dummy cycles
    temp_ccr &= ~QUADSPI_CCR_DCYC_Msk;
    temp_ccr |= (config->dummy_cycles << QUADSPI_CCR_DCYC_Pos) & QUADSPI_CCR_DCYC_Msk;

    // Single instruction mode
    temp_ccr &= ~QUADSPI_CCR_SIOO_Msk;
    temp_ccr |= (config->single_instruction << QUADSPI_CCR_SIOO_Pos) & QUADSPI_CCR_SIOO_Msk;

    QUADSPI->CCR = temp_ccr;

    return true;
}

void PHAL_qspiSetFunctionMode(QUADSPI_FunctionMode_t new_mode)
{
    uint32_t temp_ccr = QUADSPI->CCR;
    temp_ccr &= ~QUADSPI_CCR_FMODE_Msk;
    temp_ccr |= (new_mode << QUADSPI_CCR_FMODE_Pos) & QUADSPI_CCR_FMODE_Msk;
    QUADSPI->CCR = temp_ccr;
}

void PHAL_qspiSetDataWidth(QUADSPI_LineWidth_t new_width)
{
    uint32_t temp_ccr = QUADSPI->CCR;
    temp_ccr &= ~QUADSPI_CCR_DMODE_Msk;
    temp_ccr |= (new_width << QUADSPI_CCR_DMODE_Pos) & QUADSPI_CCR_DMODE_Msk;
    QUADSPI->CCR = temp_ccr;
}

void PHAL_qspiSetInstructionWidth(QUADSPI_LineWidth_t new_width)
{
    uint32_t temp_ccr = QUADSPI->CCR;
    temp_ccr &= ~QUADSPI_CCR_IMODE_Msk;
    temp_ccr |= (new_width << QUADSPI_CCR_IMODE_Pos) & QUADSPI_CCR_IMODE_Msk;
    QUADSPI->CCR = temp_ccr;
}

void PHAL_qspiSetAddressWidth(QUADSPI_LineWidth_t new_width)
{
    uint32_t temp_ccr = QUADSPI->CCR;
    temp_ccr &= ~QUADSPI_CCR_ADSIZE_Msk;
    temp_ccr |= (new_width << QUADSPI_CCR_ADSIZE_Pos) & QUADSPI_CCR_ADSIZE_Msk;
    QUADSPI->CCR = temp_ccr;
}

void PHAL_qspiSetAddressSize(QUADSPI_FieldSize_t new_size)
{
    uint32_t temp_ccr = QUADSPI->CCR;
    temp_ccr &= ~QUADSPI_CCR_ADSIZE_Msk;
    temp_ccr |= (new_size << QUADSPI_CCR_ADSIZE_Pos) & QUADSPI_CCR_ADSIZE_Msk;
    QUADSPI->CCR = temp_ccr;
}

bool PHAL_qspiWrite(uint8_t instruction, uint32_t address, uint8_t* tx_data, uint32_t tx_length)
{
    qspiWaitFree();
    QUADSPI->CR &= ~(QUADSPI_CR_DMAEN_Msk | QUADSPI_CR_EN);

    // Set instruction
    QUADSPI->CCR &= ~QUADSPI_CCR_INSTRUCTION_Msk; 
    QUADSPI->CCR |= (instruction << QUADSPI_CCR_INSTRUCTION_Pos) & QUADSPI_CCR_INSTRUCTION_Msk;
    
    tx_length = tx_length > 16 ? 16 : tx_length; // Maximum size without DMA is 16 bytes at a time.
    if (tx_length > 0)
        QUADSPI->DLR = tx_length;
    // Set Address
    QUADSPI->AR = address;

    if (tx_length > 0)
    {
        // *** Populate up FIFO ***
        // Ensure FIFO is empty
        if (QUADSPI->SR & QUADSPI_SR_FLEVEL)
            QUADSPI->CR |= QUADSPI_CR_ABORT;

        for(uint8_t i = 0; i < tx_length; i++)
            QUADSPI->DR = tx_data[i]; // Fill up fifo
    }   

    // *** Begin Transfer ***
    QUADSPI->CR |= QUADSPI_CR_EN; 
}

bool PHAL_qspiRead(uint8_t instruction, uint32_t address, uint8_t* rx_data, uint32_t rx_length)
{
    
    qspiWaitFree();
    QUADSPI->CR &= ~(QUADSPI_CR_DMAEN_Msk | QUADSPI_CR_EN);

    // Set instruction
    QUADSPI->CCR &= ~QUADSPI_CCR_INSTRUCTION_Msk;
    QUADSPI->CCR |= (instruction << QUADSPI_CCR_INSTRUCTION_Pos) & QUADSPI_CCR_INSTRUCTION_Msk;

    // Set Address
    QUADSPI->AR = address;

    // Transfer length
    QUADSPI->DLR = rx_length == 0 ?  0 : rx_length - 1;

    // *** Begin Transfer ***
    QUADSPI->CR |= QUADSPI_CR_EN;
    QUADSPI->AR = address;


    if (rx_length > 0)
    {
        
        // *** Populate up FIFO ***
        // rx_length = QUADSPI->DLR + 1U;
        while(rx_length > 0U)
        {

            while(0 == (QUADSPI->SR & QUADSPI_SR_FTF) && 0 == (QUADSPI->SR & QUADSPI_SR_TCF))
                ;

            // Only access lower 8 bits of DR to decrement FIFO occupancy by 1.
            __IO uint32_t *data_reg = &QUADSPI->DR;
            uint8_t data;
            asm ("ldrb %[dst], [%[addr], #0]" 
                : [dst] "=r" (data) 
                : [addr]"r" (data_reg)
            );
            *rx_data = (uint8_t) data;
            rx_data++;
            rx_length--;
        }
    }
    QUADSPI->CR &= ~(QUADSPI_CR_EN);
}

bool PHAL_qspiTrasnfer_DMA(uint8_t instruction, uint32_t address, uint8_t* data, uint32_t length)
{
    // DMA transfer enabled on FIFO Threshold flag
    QUADSPI->CR |= QUADSPI_CR_DMAEN;
}
