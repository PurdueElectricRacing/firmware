#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__

#include "common/phal_L4/usart/usart.h"
#include "common/common_defs/common_defs.h"
#include "common/psched/psched.h"
#include "common/queue/queue.h"
#include "source/driveline/can/can_parse.h"
#include "stm32l432xx.h"
#include "string.h"
#include "stdio.h"
#include "plettenberg.h"
#include "MC_PL0.h"
#include "common/modules/wheel_speeds/wheel_speeds.h"

#define TI_MAX_TX_LENGTH (104)
#define TI_MAX_RX_LENGTH (40)

#define TI_TIMEOUT_CONSTRAINT_TIME (1000)
#define TI_RX_LARGE_TIMEOUT_MS 1500
#define TI_RX_SMALL_TIMEOUT_MS 30
#define TI_PARSE_TIMEOUT       1500

typedef struct 
{
    // Micro status
    uint16_t      init_time;                        // Current init timing
    uint32_t      last_serial_time;                 // Last time verification of serial mode found
    uint32_t      last_parse_time;                  // Last time data was succesfully parsed
    motor_link_state_t link_state;                  // Current state of the USART connection
    motor_link_error_t last_link_error;             // Last link error

    // Micro inputs
    int      Tx_in[4];                         // Input Torque Command

    // Motor configuration
    uint32_t      rx_timeout;                       // Dynamically set timeout to determine connection status

    // Micro outputs
    uint16_t      voltage_x10;                      
    uint16_t      current_x10;
    uint16_t      curr_power_x10;                   // Last torque command percent output sent x10
    uint32_t      rpm;
    uint8_t       controller_temp;
    uint8_t       motor_temp;

    // Communications
    q_handle_t   *tx_queue;                         // FIFO for tx commands to be sent via DMA
    bool          data_stale;                       // True if data has not been parsed for MC_PARSE_TIMEOUT

    volatile uint32_t last_rx_time;                 // Time of the last rx message received
    volatile uint8_t  last_rx_loc;                  // Index of the last byte of the last byte received
    volatile uint32_t last_msg_time;                // Time of the last rx message that was large
    volatile uint8_t  last_msg_loc;                 // Index of the first byte of the last command received
    volatile char     rx_buf[TI_MAX_RX_LENGTH];     // DMA rx circular buffer
} micro_t;

void tiInit(micro_t *m, q_handle_t *tx_queue);
void tiPeriodic(micro_t *m);
void tiSetParam(float pow_left, motor_t *m, micro_t *mi, ExtU *rtU, WheelSpeeds_t *w);

#endif