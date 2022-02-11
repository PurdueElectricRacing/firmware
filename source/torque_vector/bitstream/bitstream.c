#include "main.h"
#include "bitstream.h"
#include "common/phal_L4/quadspi/quadspi.h"
#include "common/phal_L4/gpio/gpio.h"
#include "spi_flash.h"

typedef enum {
    BITSTREAM_LOCKED   = 0x0,
    BITSTREAM_UNLOCKED = 0x1,
} BITSTREAM_STATE_t;

extern void canTxUpdate();

BITSTREAM_STATE_t current_state;                   // Lock/Unlock the access to FPGA bitstream SPI flash
volatile uint16_t download_timeout_counter;        // Timeout for recieving new bitstream data
/* Download progress indicators */
uint32_t expected_bitstream_size;                  // Expected size for new bitstream data
uint32_t current_bitstream_data_addr;              // Current address for writing to bitstream
/* Data buffer for DMA transactions */
uint8_t  bitstream_buffer[2][256] = {0};           // Buffer for holding bitstream data
uint32_t bitstream_buffer_index;                   // Index for filling bitstream data
uint8_t* bitstream_active_buffer;                  // Current active buffer

bool bitstreamInit()
{
    current_state = BITSTREAM_LOCKED;
    download_timeout_counter = 0;
    bitstream_active_buffer  = 0;

    QUADSPI_Config_t mx25l3233F_spiflash_config = {
        .mode=QUADSPI_INDIRECT_READ_MODE,

        .instruction_lines=QUADSPI_SKIP_SECTION,
        .single_instruction=false,

        .address_lines=QUADSPI_SINGLE_LINE,
        .address_size=QUADSPI_24_BIT,

        .alternate_lines=QUADSPI_SKIP_SECTION, // Do not use alternate bytes section
        .alternate_size=QUADSPI_8_BIT,

        .data_lines=QUADSPI_SINGLE_LINE,

        .dummy_cycles=0,
        .fifo_threshold=0
    };

    PHAL_qspiConfigure(&mx25l3233F_spiflash_config);
    QUADSPI->CR  &= ~(QUADSPI_CR_PRESCALER_Msk);
    QUADSPI->CR  |= (10 << QUADSPI_CR_PRESCALER_Pos) & QUADSPI_CR_PRESCALER_Msk;
    QUADSPI->DCR |= (25 << QUADSPI_DCR_FSIZE_Pos);

    uint32_t id = spiFlashReadID();

    if (id != 0x20C2)
        return false;

    return true;
}

void bitstream10Hz()
{
    // Download timeout counter decrement
    if (BITSTREAM_UNLOCKED == current_state)
    {
        
        if (download_timeout_counter <= 1)
        {
            /* Timeout on rx of bitstream data */
            download_timeout_counter = 0;
            current_state = BITSTREAM_LOCKED;
            SEND_BITSTREAM_FLASH_STATUS(q_tx_can, 0, 0, 1, 0);
        }
        else
        {
            download_timeout_counter--;
        }
    }
}

/**
 * @brief Callback when new bitstream data message Rxd.
 * 
 * @param msg_data_a New RX data from CAN
 */
int num_data = 0;
void bitstream_data_CALLBACK(CanParsedData_t* msg_data_a)
{
    if (BITSTREAM_UNLOCKED == current_state)
    {
        download_timeout_counter = BITSTREAM_FLASH_RX_TIMEOUT;
        num_data += 1;
        
        // Write to temp buffer, other task will handle transfer out of buffer to QSPI
        
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d0;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d1;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d2;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d3;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d4;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d5;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d6;
        bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d7;

        if (bitstream_buffer_index >= 255 || current_bitstream_data_addr + bitstream_buffer_index >= expected_bitstream_size)
        {
            //spiFlashProgramBytes(current_bitstream_data_addr, 256, bitstream_active_buffer);
            // Swap buffers
            current_bitstream_data_addr += bitstream_buffer_index;
            bitstream_buffer_index = 0;
            if (bitstream_active_buffer == bitstream_buffer[0])
                bitstream_active_buffer =  bitstream_buffer[1];
            else
                bitstream_active_buffer = bitstream_buffer[0];
            
            if(current_bitstream_data_addr >= expected_bitstream_size)
            {
                current_state = BITSTREAM_LOCKED;
                SEND_BITSTREAM_FLASH_STATUS(q_tx_can, 0, 1, 0, 0);               
            } 
        }

        SEND_BITSTREAM_FLASH_PROGRESS(q_tx_can, current_bitstream_data_addr >> 8, bitstream_buffer_index + current_bitstream_data_addr);
    }
}

/**
 * @brief New state change request from CAN tester
 * 
 * @param msg_data_a 
 */
void bitstream_request_CALLBACK(CanParsedData_t* msg_data_a)
{
    // Check if it is safe to download a new bitstream and change states if so
    if (1 == msg_data_a->bitstream_request.download_request && 
        BITSTREAM_LOCKED == current_state
       )
    {
        current_state = BITSTREAM_UNLOCKED;
        download_timeout_counter = BITSTREAM_FLASH_RX_TIMEOUT;
        expected_bitstream_size = msg_data_a->bitstream_request.download_size;
        bitstream_buffer_index = 0;
        current_bitstream_data_addr = 0;
        bitstream_active_buffer = bitstream_buffer[0];

        PHAL_writeGPIO(FPGA_CFG_RST_GPIO_Port, FPGA_CFG_RST_Pin, 0);
        PHAL_writeGPIO(QUADSPI_CS_FPGA_GPIO_Port, QUADSPI_CS_FPGA_Pin, 1);
        PHAL_writeGPIO(QUADSPI_IO3_GPIO_Port, QUADSPI_IO3_Pin, 1);        

        // for (int i = 0; i < 304; i++)
        // {
        //     if (!spiFlashSectorErase(i))
        //         if (i > 0)
        //             i--;
        // }
        SEND_BITSTREAM_FLASH_STATUS(q_tx_can, 1, 0, 0, 0);
        SEND_BITSTREAM_FLASH_PROGRESS(q_tx_can, 0, 0);
    }
}
