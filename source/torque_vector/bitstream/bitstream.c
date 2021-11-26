#include "main.h"
#include "bitstream.h"
#include "common/phal_L4/quadspi/quadspi.h"
#include "common/phal_L4/gpio/gpio.h"

typedef enum {
    BITSTREAM_LOCKED   = 0x0,
    BITSTREAM_UNLOCKED = 0x1,
} BITSTREAM_STATE_t;

#define BITSTEAM_BUFFER_SIZE (QUADSPI_FIFO_SIZE_BYTES)


BITSTREAM_STATE_t current_state;                   // Lock/Unlock the access to FPGA bitstream SPI flash
uint16_t download_timeout_counter;                 // Timeout for recieving new bitstream data
uint32_t expected_bitstream_size;                  // Expected size for new bitstream data
uint8_t  bitstream_buffer[2][BITSTEAM_BUFFER_SIZE];// Buffer for holding bitstream data
uint8_t  bitstream_buffer_index;                   // Index for filling bitstream data
uint8_t* bitstream_active_buffer;                  // Current active buffer

void bitstreamInit()
{
    current_state = BITSTREAM_LOCKED;
    download_timeout_counter = 0;
    bitstream_active_buffer  = 0;

    QUADSPI_Config_t mx25l3233F_spiflash_config = {
        .mode=QUADSPI_INDIRECT_READ_MODE,

        .instruction_lines=QUADSPI_SKIP_SECTION,
        .single_instruction=true,

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
}

uint32_t delay = 0;
uint8_t data[8] = {0};
void bitstream10Hz()
{
    
    if (delay++ == 10)
    {
        PHAL_writeGPIO(QUADSPI_IO3_GPIO_Port, QUADSPI_IO3_Pin, 1);
        QUADSPI->CR |= QUADSPI_CR_SSHIFT;
        
        PHAL_qspiRead(0x00, 0x9F, data, 8);
        asm("bkpt");
    }

    // Download timeout counter decrement
    if (BITSTREAM_UNLOCKED == current_state)
    {
        download_timeout_counter--;
        if (download_timeout_counter <= 0)
        {
            /* Timeout on rx of bitstream data */
            download_timeout_counter = 0;
            current_state = BITSTREAM_LOCKED;
            CanMsgTypeDef_t msg = {.ExtId=ID_BITSTREAM_FLASH_STATUS, .DLC=DLC_BITSTREAM_FLASH_STATUS, .IDE=1};
            CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;
            data_a->bitstream_flash_status.flash_timeout_rx = 1;

            qSendToBack(&q_tx_can, &msg);
        }
    }
    else
    {
        // Read SPIFlash status register
    }
}

void bitstream100Hz()
{
    // Download timeout counter decrement
    if (BITSTREAM_UNLOCKED == current_state)
    {
        if (BITSTEAM_BUFFER_SIZE == bitstream_buffer_index)
        {
            // Swap buffers
            bitstream_buffer_index = 0;
            if (bitstream_active_buffer == (uint8_t*) &bitstream_buffer[0])
                bitstream_active_buffer = (uint8_t*) &bitstream_buffer[1];
            else
                bitstream_active_buffer = (uint8_t*) &bitstream_buffer[0];

            // Begin transfer
            PHAL_qspiWrite(0x00, 0x00, bitstream_active_buffer, QUADSPI_FIFO_SIZE_BYTES);
        }
    }
}

/**
 * @brief Callback when new bitstream data message Rxd.
 * 
 * @param msg_data_a New RX data from CAN
 */
void bitstream_data_IRQ(CanParsedData_t* msg_data_a)
{
    if (BITSTREAM_UNLOCKED == current_state)
    {
        if (BITSTEAM_BUFFER_SIZE > bitstream_buffer_index+8)
        {
            // Write to temp buffer, other task will handle transfer out of buffer to QSPI
            download_timeout_counter = BITSTREAM_FLASH_RX_TIMEOUT;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d0;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d1;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d2;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d3;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d4;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d5;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d6;
            bitstream_active_buffer[bitstream_buffer_index++] = (uint8_t) msg_data_a->bitstream_data.d7;
        }
        else
        {
            // OVERRUN ERROR, CAN Message RX before buffer swap.
            CanMsgTypeDef_t msg = {.ExtId=ID_BITSTREAM_FLASH_STATUS, .DLC=DLC_BITSTREAM_FLASH_STATUS, .IDE=1};
            CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;
            data_a->bitstream_flash_status.flash_timeout_rx = 1;

            qSendToBack(&q_tx_can, &msg);
        }
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
        // && car.state == pre_ready_to_drive
        )
    {
        current_state = BITSTREAM_UNLOCKED;
        download_timeout_counter = BITSTREAM_FLASH_RX_TIMEOUT;
        expected_bitstream_size = msg_data_a->bitstream_request.download_size;
        bitstream_buffer_index = 0;
        // TODO hold FPGA in reset
        // TODO Erase entire SPI flash
    }
}
