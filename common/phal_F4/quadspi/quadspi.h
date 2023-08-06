#ifndef _PHAL_QUADSPI_H
#define _PHAL_QUADSPI_H

#include "stm32l4xx.h"
#include <stdbool.h>

#define QUADSPI_FIFO_SIZE_BYTES (16)


typedef enum {
    QUADSPI_INDIRECT_WRITE_MODE = 0x00,
    QUADSPI_INDIRECT_READ_MODE  = 0x01,
    QUADSPI_AUTOMATIC_POLL_MODE = 0x10,
} QUADSPI_FunctionMode_t;

typedef enum {
    QUADSPI_SKIP_SECTION= 0x00, /* Skip over this phase of transfer */
    QUADSPI_SINGLE_LINE = 0x01, /* Use single QSPI line for transfer */
    QUADSPI_DUAL_LINE   = 0x10, /* Use two QSPI lines for transfer */
    QUADSPI_QUAD_LINE   = 0x11, /* Use four QSPI lines for transfer */
} QUADSPI_LineWidth_t;

typedef enum {
    QUADSPI_8_BIT  = 0x00,
    QUADSPI_16_BIT = 0x01,
    QUADSPI_24_BIT = 0x10,
    QUADSPI_32_BIT = 0x11,
} QUADSPI_FieldSize_t;

typedef struct{
    QUADSPI_FunctionMode_t mode;            /* Functional mode selection */
    
    QUADSPI_LineWidth_t instruction_lines;  /* Number of SPI lines to use for instruction transfer */
    bool single_instruction;                /* Only send instructions once, might not be supported by device. */

    QUADSPI_LineWidth_t address_lines;      /* Number of SPI lines to use for address transfer */
    QUADSPI_FieldSize_t address_size;       /* Number of bytes to use for data transfer */
    
    QUADSPI_LineWidth_t alternate_lines;    /* Number of SPI lines to use for address transfer */
    QUADSPI_FieldSize_t alternate_size;     /* Number of bytes to use for address transfer */

    QUADSPI_LineWidth_t data_lines;         /* Number of SPI lines to use for data transfer */
    
    uint8_t dummy_cycles;                   /* Number of dummy cycles to insert after address */
    uint8_t fifo_threshold;                 /* Number of bytes to initiate the FIFO Threshold flag */
} QUADSPI_Config_t;

/**
 * @brief Initilize quadspi peripheral clocks and speed to defaults
 * 
 * @return true  Successfuly initilized QUADSPI peripheral
 * @return false 
 */
bool PHAL_qspiInit();

/**
 * @brief Configure QUADSPI peripheral for SPI Flash transactions
 *        Will setup flash size, dummy cycles, instruction width, and data width.
 * 
 * @return true 
 * @return false 
 */
bool PHAL_qspiConfigure(QUADSPI_Config_t* config);

bool PHAL_qspiTrasnfer(uint8_t instruction, uint32_t address, uint8_t* data, uint32_t length);

#endif