#include "bitstream.h"

typedef enum {
    BITSTREAM_LOCKED   = 0x0,
    BITSTREAM_UNLOCKED = 0x1,
} BITSTREAM_STATE_t;


BITSTREAM_STATE_t current_state;        // Lock/Unlock the access to FPGA bitstream SPI flash
uint16_t download_timeout_counter;      // Timeout for recieving new bitstream data
uint32_t expected_bitstream_size;       // Expected size for new bitstream data

void bitstream_init()
{
    current_state = BITSTREAM_LOCKED;
    download_timeout_counter = 0;
}

void bitstream_10Hz()
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

/**
 * @brief Callback when new bitstream data message Rxd.
 * 
 * @param msg_data_a New RX data from CAN
 */
void bitstream_data_IRQ(CanParsedData_t* msg_data_a)
{
    // Begin SPI transaction if we are in the right state
    if (BITSTREAM_UNLOCKED == current_state)
    {
        download_timeout_counter = BITSTREAM_FLASH_RX_TIMEOUT;
        msg_data_a->bitstream_data.word_0;
        msg_data_a->bitstream_data.word_1;
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
        // hold FPGA in reset
    }
}
