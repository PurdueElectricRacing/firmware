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
    ETHERNET_STATE_HW_INIT    = 0,
    ETHERNET_STATE_UDP_INIT   = 1,
    ETHERNET_STATE_LINKING    = 2,
    ETHERNET_STATE_READY2TX   = 3,
    ETHERNET_STATE_RECOVERING = 4,
    ETHERNET_STATE_FATAL      = 5,
} ethernet_state_t;

typedef struct {
    wiz_NetInfo net_info;
    uint8_t udp_address[4];
    uint16_t udp_port;
    uint8_t udp_socket;
} ethernet_config_t;

void w5500_register_callbacks(void);
void ethernet_periodic(void);

#endif // ETHERNET_H