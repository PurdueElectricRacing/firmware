/**
 * @file daq_eth.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 *         Eileen Yoon (eyn@purdue.edu)
 * @brief  W5500 Ethernet
 *
 * @version 0.1
 * @date 2025-02-25
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <string.h>

#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "daq_can.h"
#include "daq_hub.h"
#include "main.h"
#include "w5500/socket.h"
#include "w5500/wizchip_conf.h"
#include "common/can_library/generated/DAQ.h"

static int8_t eth_init(void);
static int8_t eth_get_link_up(void);
static int8_t eth_udp_init(void);
static void eth_udp_send_periodic(void);
static void eth_reset_error(void);
static void _eth_handle_error(eth_error_t err, int32_t reason);
#define eth_handle_error(err, res) _eth_handle_error(err, res)

typedef struct
{
    wiz_NetInfo net_info;
    uint8_t udp_bc_addr[4];
    uint16_t udp_bc_port;
    uint8_t udp_bc_sock;
    uint16_t tcp_port;
    uint8_t tcp_sock;
} eth_config_t;

eth_config_t eth_config = {
    .net_info = {
        .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // Mac address
        .ip  = {192, 168, 10, 41}, // IP address of DAQ PCB
        .sn  = {255, 255, 255, 0}, // Subnet mask
        .gw  = {192, 168, 10, 1}, // Gateway address
    },
    .udp_bc_addr = {192, 168, 10, 255}, // Broadcast address
    .udp_bc_port = 5005,
    .udp_bc_sock = DAQ_SOCKET_UDP_BROADCAST,
    .tcp_port    = 5005,
    .tcp_sock    = DAQ_SOCKET_TCP,
};

void eth_update_periodic(void) {
    static uint8_t eth_startup = 0;
    if (!eth_startup) {
        mDelay(250); // Wait for module to kick up
        eth_startup = 1;
    }

    switch (daq_hub.eth_state) {
        case ETH_LINK_IDLE:
            if (eth_init() == ETH_ERROR_NONE) {
                // Chip initialized, setup UDP socket
                if (eth_udp_init() == ETH_ERROR_NONE)
                    daq_hub.eth_state = ETH_LINK_STARTING;
                else
                    daq_hub.eth_state = ETH_LINK_FAIL;
            } else {
                daq_hub.eth_state = ETH_LINK_FAIL;
            }
            break;
        case ETH_LINK_DOWN: // intentional fall through
        case ETH_LINK_STARTING:
            if (eth_get_link_up()) {
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
                debug_printf("UDP UP!\n");
                daq_hub.eth_state = ETH_LINK_UP;
            }
            break;
        case ETH_LINK_UP:
            if (eth_get_link_up()) {
                daq_hub.eth_state = ETH_LINK_UP;
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
                /* Eth link UP routines */
                eth_udp_send_periodic();
            } else {
                daq_hub.eth_state = ETH_LINK_DOWN;
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
            }
            break;
        case ETH_LINK_FAIL:
            // Stay in fail state until eth_reset_error() moves us out
            break;
    }

    eth_reset_error();
}

static void _eth_handle_error(eth_error_t err, int32_t reason) {
    ++daq_hub.eth_error_ct;
    daq_hub.eth_last_err        = err;
    daq_hub.eth_last_err_res    = reason;
    daq_hub.eth_last_error_time = getTick();
    PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 1);
    debug_printf("eth err: %d res: %d\n", err, reason);
}

static void eth_reset_error(void) {
    // Do not retry immediately
    if (!(getTick() - daq_hub.eth_last_error_time > ETH_ERROR_RETRY_MS))
        return;

    if (daq_hub.eth_tcp_state == ETH_TCP_FAIL) {
        daq_hub.eth_tcp_state    = ETH_TCP_IDLE; // Retry TCP only
        daq_hub.eth_last_err     = ETH_ERROR_NONE;
        daq_hub.eth_last_err_res = 0;
        PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
    } else if (daq_hub.eth_last_err != ETH_ERROR_NONE || daq_hub.eth_state == ETH_LINK_FAIL) {
        daq_hub.eth_state        = ETH_LINK_IDLE; // Retry
        daq_hub.eth_last_err     = ETH_ERROR_NONE;
        daq_hub.eth_last_err_res = 0;
        PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
    }

    if (daq_hub.eth_state == ETH_LINK_UP) {
        if (daq_hub.eth_tcp_state == ETH_TCP_ESTABLISHED)
            PHAL_toggleGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN); // Blink on TCP
        else
            PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
    } else {
        PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
    }
}

/* UDP */
static int8_t eth_init(void) {
    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 0);
    osDelay(ETH_PHY_RESET_PERIOD_MS);
    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 1);
    osDelay(ETH_PHY_RESET_PERIOD_MS); // datasheet says 50 ms

    // Check for sign of life
    uint8_t ret = getVERSIONR();
    if (ret != (uint8_t)ETH_PHY_VERSION_ID) {
        eth_handle_error(ETH_ERROR_VERS, ret);
        return ETH_ERROR_VERS;
    }

    // W5500 has 16kB memory for RX/TX each internally. It doesnt matter how the block is distributed
    // across its 8 internal sockets, so allocate all 16kB to the sockets we actually use.
    // 0: UDP broadcast (don't need RX buffer, need big TX)
    // 1: TCP/UDS (need big RX buffer for bootloader, only need small TX for ack messages)
    // 2/3/4: FTP
    uint8_t rxBufSizes[] = {8, 8, 0, 0, 0, 0, 0, 0}; // kB
    uint8_t txBufSizes[] = {8, 8, 0, 0, 0, 0, 0, 0}; // kB

    if (wizchip_init(txBufSizes, rxBufSizes)) {
        eth_handle_error(ETH_ERROR_INIT, 0);
        return ETH_ERROR_INIT;
    }

    wizchip_setnetinfo(&eth_config.net_info);

    return ETH_ERROR_NONE;
}

static int8_t eth_get_link_up(void) {
    volatile uint8_t phycfgr = getPHYCFGR();
    return phycfgr & PHY_LINK_ON;
}

static int8_t eth_udp_init(void) {
    // UDP Broadcast address
    // 255.255.255.255 is not forwarded by routers
    // local address | (~subnet address) is forwarded
    int8_t bc_sock;
    bc_sock = socket(eth_config.udp_bc_sock, SOCK_DGRAM, eth_config.udp_bc_port, 0);
    if (bc_sock != eth_config.udp_bc_sock) {
        eth_handle_error(ETH_ERROR_UDP_SOCK, bc_sock);
        return ETH_ERROR_UDP_SOCK;
    }

    return ETH_ERROR_NONE;
}

static void eth_udp_send_periodic(void) {
    int32_t ret;
    timestamped_frame_t* buf;
    uint32_t consecutive_items;

    if (daq_hub.eth_state == ETH_LINK_UP) {
        if (SPMC_follower_pop(&queue, &buf, &consecutive_items) == 0) {
            if (consecutive_items > UDP_MAX_WRITE_COUNT)
            
                consecutive_items = UDP_MAX_WRITE_COUNT; // limit
            // Write time :D
            ret = sendto(eth_config.udp_bc_sock, (uint8_t*)buf, consecutive_items * sizeof(*buf), eth_config.udp_bc_addr, eth_config.udp_bc_port);
            if (ret < consecutive_items * sizeof(*buf)) {
                eth_handle_error(ETH_ERROR_UDP_SEND, ret);
            } 
        }
    }
}

// static void eth_udp_send_frame(timestamped_frame_t* frame) {
//     timestamped_frame_t* rx; // TODO check if this is safe (two producers)
//     uint32_t cont;
//     if (bGetHeadForWrite(&b_rx_can, (void**)&rx, &cont) == 0) {
//         memcpy(rx, frame, sizeof(*frame));
//         bCommitWrite(&b_rx_can, 1); // Add it to regular CAN RX queue that DAQ broadcasts
//     }
// }
