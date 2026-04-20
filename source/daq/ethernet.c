/**
 * @file ethernet.c
 * @brief Implementation of the ethernet thread for wireless data acquisition.
 *
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include "w5500/socket.h"
#include "common/phal/gpio.h"
#include "common/freertos/freertos.h"
#include "main.h"
#include "spmc.h"

#include "ethernet.h"

static constexpr uint8_t DAQ_UDP_SOCKET_NUM = 0;
static_assert(
    DAQ_UDP_SOCKET_NUM < _WIZCHIP_SOCK_NUM_,
    "UDP socket number should be less than WIZCHIP socket max"
);

ethernet_config_t config = {
    .net_info = {
        .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // Mac address
        .ip  = {192, 168, 10, 41}, // IP address of DAQ PCB
        .sn  = {255, 255, 255, 0}, // Subnet mask
        .gw  = {192, 168, 10, 1},  // Gateway address
    },
    .udp_address = {192, 168, 10, 255}, // Broadcast address
    .udp_port = 5005,
    .udp_socket = DAQ_UDP_SOCKET_NUM
};

static eth_thread_state_t current_state = ETH_THREAD_HW_INIT;
static eth_thread_state_t next_state = ETH_THREAD_HW_INIT;

static inline bool is_linked() {
    volatile uint8_t phycfgr = getPHYCFGR();
    return phycfgr & PHY_LINK_ON;
}

static bool init_w5500() {
    static constexpr uint8_t EXPECTED_W5500_VERSION_ID = 0x04;

    // hold and release ETH reset pin
    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 0);
    osDelay(10);
    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 1);
    osDelay(50);

    // Read back the version register
    if (getVERSIONR() != EXPECTED_W5500_VERSION_ID) {
        return false;
    }

    // W5500 has 16kB memory for RX/TX each internally. It doesnt matter how the block is distributed
    // across its 8 internal sockets, so allocate all 16kB to the sockets we actually use.
    // 0: UDP broadcast (don't need RX buffer, need big TX)
    // 1: TCP/UDS (need big RX buffer for bootloader, only need small TX for ack messages)
    // 2/3/4: FTP
    uint8_t rxBufSizes[] = {8, 8, 0, 0, 0, 0, 0, 0}; // kB
    uint8_t txBufSizes[] = {8, 8, 0, 0, 0, 0, 0, 0}; // kB

    if (wizchip_init(txBufSizes, rxBufSizes)) {
        return false;
    }

    wizchip_setnetinfo(&config.net_info);

    return true;
}

static bool init_udp(void) {
    // UDP Broadcast address
    // 255.255.255.255 is not forwarded by routers
    // local address | (~subnet address) is forwarded
    int8_t bc_sock = socket(config.udp_socket, SOCK_DGRAM, config.udp_port, 0);
    if (bc_sock != config.udp_socket) {
        return false;
    }

    return true;
}

static void broadcast_udp(void) {
    static constexpr size_t UDP_MAX_BUFFER_SIZE = 8192;
    static constexpr size_t UDP_MAX_WRITE_COUNT = UDP_MAX_BUFFER_SIZE / sizeof(timestamped_frame_t);
    static constexpr size_t UDP_MAX_WRITE_CHUNKS = UDP_MAX_WRITE_COUNT / SPMC_CHUNK_NUM_FRAMES;

    timestamped_frame_t* outgoing;
    size_t chunks_available = SPMC_follower_peek_chunks(&spmc, &outgoing);
    if (chunks_available == 0) {
        return; // No data to send
    }

    if (chunks_available > UDP_MAX_WRITE_CHUNKS) {
        chunks_available = UDP_MAX_WRITE_CHUNKS; // Cap to max UDP size
    }

    // Write time :D
    uint16_t write_len = SPMC_CHUNK_NUM_FRAMES * chunks_available * sizeof(timestamped_frame_t);
    int32_t ret = sendto(config.udp_socket, (uint8_t*)outgoing, write_len, config.udp_address, config.udp_port);
    if (ret < write_len) {
        // todo handle this error
        return;
    } 

    SPMC_follower_advance_tail(&spmc, chunks_available);
}

void eth_thread_periodic() {
    current_state = next_state;
    next_state = current_state; // default to no state change

    switch (current_state) {
        case ETH_THREAD_HW_INIT:
            osDelay(200); // block for a bit between each init attempt
            
            if (init_w5500()) {
                next_state = ETH_THREAD_UDP_INIT;
            }
            break;
        case ETH_THREAD_UDP_INIT:
            osDelay(200); // block for a bit between each init attempt

            if (init_udp()) {
                next_state = ETH_THREAD_LINKING;
            }
            break;
        case ETH_THREAD_LINKING:
            osDelay(100); // block for a bit between each link check

            if (is_linked()) {
                next_state = ETH_THREAD_READY;
            }
            break;
        case ETH_THREAD_READY:
            // todo sleep until woken by an ISR, then begin TX
            // xNotifyTake(, portMAX_DELAY);

            if (!is_linked()) {
                next_state = ETH_THREAD_LINKING; // Link lost, try to relink
            } else {
                next_state = ETH_THREAD_TXING;
            }
            break;
        case ETH_THREAD_TXING:
            broadcast_udp();

            next_state = ETH_THREAD_READY;
            break;
        case ETH_THREAD_RECOVERING:
            // todo timeout or error during TX
            break;
        case ETH_THREAD_FATAL:
            // todo after a couple of tries, give up and wait for a reset
            break;
    }
}


