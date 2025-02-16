/**
 * @file daq_hub.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Data acquisition from CAN to:
 *          - SD Card
 *          - Ethernet
 *          - USB (maybe)
 * @version 0.1
 * @date 2024-02-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _DAQ_HUB_H_
#define _DAQ_HUB_H_

#include <stdint.h>
#include <assert.h>

#include "common/freertos/freertos.h"
#include "can_flags.h"
#include "ff.h"
#include "sdio.h"

// W5500 has 8 sockets internally
#define DAQ_SOCKET_UDP_BROADCAST 0
#define DAQ_SOCKET_TCP           1
#define DAQ_SOCKET_FTP_CTRL0     2  // FTP uses 3 sockets
#define DAQ_SOCKET_FTP_DATA      3
#define DAQ_SOCKET_FTP_CTRL1     4

#define ETH_PHY_RESET_PERIOD_MS 10

typedef uint32_t canid_t;
typedef uint8_t busid_t;

typedef enum
{
    SD_STATE_IDLE         = 0,
    SD_STATE_MOUNTED      = 1,
    SD_STATE_FILE_CREATED = 2,
    SD_FAIL               = 3,
    SD_SHUTDOWN           = 4,
} sd_state_t;

typedef enum
{
    SD_ERROR_NONE        = 0,
    SD_ERROR_MOUNT       = 1,
    SD_ERROR_FOPEN       = 2,
    SD_ERROR_FCLOSE      = 3,
    SD_ERROR_WRITE       = 4,
    SD_ERROR_DETEC       = 5,
    SD_ERROR_SYNC        = 6,
} sd_error_t;

typedef enum
{
    ETH_IDLE             = 0,
    ETH_LINK_DOWN        = 1,
    ETH_LINK_UP          = 2,
    ETH_FAIL             = 3,
} eth_state_t;

typedef enum
{
    ETH_TCP_IDLE         = 0,
    ETH_TCP_LISTEN       = 1,
    ETH_TCP_ESTABLISHED  = 2,
    ETH_TCP_FAIL         = 3,
} eth_tcp_state_t;

typedef enum
{
    ETH_ERROR_NONE       = 0,
    ETH_ERROR_INIT       = 1,
    ETH_ERROR_VERS       = 2,
    ETH_ERROR_UDP_SOCK   = 3,
    ETH_ERROR_UDP_SEND   = 4,
    ETH_ERROR_TCP_SOCK   = 5,
    ETH_ERROR_TCP_LISTEN = 6,
    ETH_ERROR_TCP_SEND   = 7,
} eth_error_t;

typedef enum
{
    TCP_CMD_HANDSHAKE = 0,
    TCP_CMD_CAN_FRAME = 1, // authentic CAN frame, i.e. daq can't TX CAN to itself
    TCP_CMD_UDS_FRAME = 2,
} tcp_cmd_t;

typedef enum __attribute__ ((__packed__))
{
    DAQ_FRAME_CAN_RX  = 0, //!< RX to DAQ over CAN
    DAQ_FRAME_TCP2CAN = 1, //!< RX to DAQ over TCP, relay to other nodes on CAN
    DAQ_FRAME_TCP2DAQ = 2, //!< RX to DAQ over TCP, message intended for DAQ
    DAQ_FRAME_TCP_TX  = 3, //!< TX from DAQ over TCP
    DAQ_FRAME_UDP_TX  = 4, //!< TX from DAQ over UDP
} daq_frame_type_t;
static_assert(sizeof(daq_frame_type_t) == sizeof(uint8_t));

typedef struct __attribute__((packed))
{
    uint8_t  frame_type;   //!< daq_frame_type_t
    uint32_t tick_ms;      //!< ms timestamp of reception
    canid_t  msg_id;       //!< message id
    busid_t  bus_id;       //!< bus the message was rx'd on
    uint8_t  dlc;          //!< data length code
    uint8_t  data[8];      //!< message data
} timestamped_frame_t;

typedef struct
{
    // Ethernet
    eth_state_t eth_state;
    eth_tcp_state_t eth_tcp_state;
    bool eth_enable_udp_broadcast; // TODO: determine if I want this var
    bool eth_enable_tcp_reception;
    uint32_t eth_error_ct;
    eth_error_t eth_last_err;
    int32_t eth_last_err_res;
    uint32_t eth_last_error_time;

    // SD Card
    sd_state_t sd_state;
    FATFS fat_fs;
    uint32_t sd_error_ct;
    sd_error_t sd_last_err;
    FRESULT sd_last_err_res;
    uint32_t sd_last_error_time;

    FIL log_fp;
    bool ftp_busy;
    uint32_t log_start_ms;
    uint32_t last_write_ms;
    uint32_t last_file_tick;
    bool log_enable_sw; //!< Debounced switch state
    bool log_enable_tcp;
    bool log_enable_uds;
} daq_hub_t;

extern daq_hub_t dh;

void daq_catch_error(void);
void daq_hub_init(void);
void daq_create_threads(void);
void uds_receive_periodic(void);
void shutdown(void);
void daq_shutdown_hook(void);

#endif
