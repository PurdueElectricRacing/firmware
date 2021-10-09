#include "bitstream.h"
#include "common/phal_L4/quadspi/quadspi.h"

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
}

void bitstream10Hz()
{
    // Download timeout counter decrement
    if (BITSTREAM_UNLOCKED == current_state)
    {
        download_timeout_counter--;
        if (download_timeout_counter <= 0)
        {
            download_timeout_counter = 0;
            current_state = BITSTREAM_LOCKED;
            // Timeout on download
            // TODO Send bitstream_flash_status signal
        }
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
            PHAL_qspiTrasnfer(0x00, 0x00, bitstream_active_buffer, QUADSPI_FIFO_SIZE_BYTES);
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
    if (msg_data_a->bitstream_request.download_request && 
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
