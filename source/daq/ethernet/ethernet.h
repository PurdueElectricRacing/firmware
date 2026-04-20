#ifndef ETHERNET_H
#define ETHERNET_H

/**
 * @file ethernet.h
 * @brief Implementation of the ethernet thread for wireless data acquisition.
 *
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include "w5500/wizchip_conf.h"

typedef enum {
    ETH_THREAD_HW_INIT    = 0,
    ETH_THREAD_UDP_INIT   = 1,
    ETH_THREAD_LINKING    = 2,
    ETH_THREAD_READY      = 3,
    ETH_THREAD_TXING      = 4,
    ETH_THREAD_RECOVERING = 5,
    ETH_THREAD_FATAL      = 6,
} eth_thread_state_t;

typedef struct {
    wiz_NetInfo net_info;
    uint8_t udp_address[4];
    uint16_t udp_port;
    uint8_t udp_socket;
} ethernet_config_t;

void w5500_register_callbacks(void);
void eth_thread_periodic();

#endif // ETHERNET_H