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

#include <assert.h>
#include <stdint.h>

#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "daq_eth.h"
#include "daq_sd.h"
#include "ff.h"
#include "sdio.h"

typedef uint32_t canid_t;
typedef uint8_t busid_t;

typedef struct __attribute__((packed)) {
    uint8_t frame_type; //!< daq_frame_type_t
    uint32_t tick_ms; //!< ms timestamp of reception
    canid_t msg_id; //!< message id
    busid_t bus_id; //!< bus the message was rx'd on
    uint8_t dlc; //!< data length code
    uint8_t data[8]; //!< message data
} timestamped_frame_t;

typedef struct
{
    // Ethernet
    eth_state_t eth_state;
    eth_tcp_state_t eth_tcp_state;
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
    uint32_t last_file_ms;
    bool log_enable_sw; //!< Debounced switch state
    bool log_enable_tcp;
    bool log_enable_uds;

    uint32_t bcan_rx_overflow;
    uint32_t can1_rx_overflow;
    uint32_t sd_rx_overflow;
    uint32_t tcp_tx_overflow;
} daq_hub_t;

extern daq_hub_t daq_hub;

void daq_hub_init(void);
void daq_create_threads(void);
void uds_receive_periodic(void);
void daq_shutdown_hook(void);
void rtc_config_cb(CanMsgTypeDef_t*);

#endif // _DAQ_HUB_H_
