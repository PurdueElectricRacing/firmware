/**
 * @file daq_hub.c
 * @author Luke Oxley (lcoxley@purdue.edu)
 *         Eileen Yoon (eyn@purdue.edu)
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

#include "daq_hub.h"

#include <string.h>

#include "common/phal/gpio.h"
#include "daq_can.h"
#include "daq_eth.h"
#include "daq_sd.h"
#include "main.h"

daq_hub_t daq_hub;

// Local protoptypes
static void daq_heartbeat(void);
static void can_send_periodic(void);

defineThreadStack(daq_heartbeat, 500, osPriorityNormal, 128); // HB
defineThreadStack(sd_update_periodic, 100, osPriorityNormal, 4096); // SD WRITE
defineThreadStack(eth_update_periodic, 50, osPriorityNormal, 4096); // SD WRITE
defineThreadStack(can_send_periodic, 50, osPriorityNormal, 128); // CAN1 TX

//defineThreadStack(uds_receive_periodic, 50, osPriorityHigh, 2048); // DAQ CAN RX

void daq_hub_init(void) {
    // Ethernet
    daq_hub.eth_state           = ETH_LINK_IDLE;
    daq_hub.eth_tcp_state       = ETH_TCP_IDLE;
    daq_hub.eth_error_ct        = 0;
    daq_hub.eth_last_error_time = 0;
    daq_hub.eth_last_err        = ETH_ERROR_NONE;
    daq_hub.eth_last_err_res    = 0;

    // SD Card
    daq_hub.sd_state           = SD_STATE_IDLE;
    daq_hub.sd_error_ct        = 0;
    daq_hub.sd_last_error_time = 0;
    daq_hub.sd_last_err        = SD_ERROR_NONE;
    daq_hub.sd_last_err_res    = 0;

    daq_hub.last_file_ms   = 0;
    daq_hub.last_write_ms  = 0;
    daq_hub.log_enable_sw  = false;
    daq_hub.log_enable_tcp = false;
    daq_hub.log_enable_uds = false;
    daq_hub.ftp_busy       = false;

    daq_hub.bcan_rx_overflow = 0;
    daq_hub.can1_rx_overflow = 0;
    daq_hub.sd_rx_overflow   = 0;
    daq_hub.tcp_tx_overflow  = 0;
}

void daq_create_threads(void) {
    createThread(daq_heartbeat); // HB
    createThread(sd_update_periodic); // SD WRITE
    // createThread(eth_update_periodic); // SD WRITE
    createThread(can_send_periodic); // CAN1 TX
    //createThread(uds_receive_periodic); // DAQ CAN RX
}

static void daq_heartbeat(void) {
    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
    CAN_SEND_daq_can_stats(can_stats.can_peripheral_stats[CAN1_IDX].tx_of, can_stats.can_peripheral_stats[CAN1_IDX].tx_fail, can_stats.rx_of, can_stats.can_peripheral_stats[CAN1_IDX].rx_overrun);
    if (daq_hub.bcan_rx_overflow || daq_hub.can1_rx_overflow || daq_hub.sd_rx_overflow || daq_hub.tcp_tx_overflow) {
        CAN_SEND_daq_queue_stats(daq_hub.bcan_rx_overflow, daq_hub.can1_rx_overflow, daq_hub.sd_rx_overflow, daq_hub.tcp_tx_overflow); // TODO reset & only send once?
    }
}

static void can_send_periodic(void) {
    CAN_tx_update();
    CAN_rx_update();
}

void uds_frame_send(uint64_t data) {
#if 0
    timestamped_frame_t frame = {.frame_type = DAQ_FRAME_UDP_TX, .tick_ms = getTick(), .msg_id = ID_UDS_RESPONSE_DAQ, .bus_id = BUS_ID_CAN1, .dlc = 8 };
    frame.msg_id |= CAN_EFF_FLAG;
    memcpy(frame.data, (uint8_t *)&data, sizeof(uint64_t));

    SEND_UDS_RESPONSE_DAQ(data);
    eth_tcp_send_frame(&frame);
    //eth_udp_send_frame(&frame); // dont send for now
#endif
}

/**
 * Pull UDS CAN frames out of UDS queue added during CAN1/CAN2 ISR
 * and process them in non-interrupt context
 */
void uds_receive_periodic(void) {
#if 0
    timestamped_frame_t rx_msg;
    while (xQueueReceive(q_can1_rx, &rx_msg, portMAX_DELAY) == pdPASS)
    {
        CanParsedData_t *msg_data_a = (CanParsedData_t *) &rx_msg.data;
        uds_command_daq_CALLBACK(msg_data_a->uds_command_daq.payload);
    }
#endif
}

// bank->BSRR |= 1 << ((!value << 4) | pin);
#define GPIO_CLEAR_BIT(PIN) ((1 << ((1 << 4) | (PIN))))

void daq_shutdown_hook(void) {
    // First, turn off all power consuming devices to increase our write time to sd card
    // To whoever is doing future DAQ rev: also change the GPIO ports here
    // all LEDs go bye bye
    GPIOD->BSRR |= GPIO_CLEAR_BIT(HEARTBEAT_LED_PIN) | GPIO_CLEAR_BIT(CONNECTION_LED_PIN) | GPIO_CLEAR_BIT(ERROR_LED_PIN);
    GPIOA->BSRR |= GPIO_CLEAR_BIT(SD_DETECT_LED_PIN) | GPIO_CLEAR_BIT(SD_ACTIVITY_LED_PIN) | GPIO_CLEAR_BIT(SD_ERROR_LED_PIN);

    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 0);
    PHAL_deinitCAN(CAN1);
    PHAL_deinitCAN(CAN2);

    sd_shutdown();
    // Hooray, we made it, blink an LED to show the world
    PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
}
