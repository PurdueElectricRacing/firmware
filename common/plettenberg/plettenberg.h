#ifndef __PLETTENBERG_H__
#define __PLETTENBERG_H__

#include "common/common_defs/common_defs.h"
#include "common/phal_L4/usart/usart.h"
#include "common/psched/psched.h"
#include "common/queue/queue.h"
#include "string.h"
#include <stdbool.h>

#define MC_MAX_TX_LENGTH (25)
#define MC_MAX_RX_LENGTH (77 + MC_MAX_TX_LENGTH)

// Connection times
#define MC_LOOP_DT (15) // Periodic update rate of motor controller task

#define MC_RX_LARGE_TIMEOUT_MS 1500 // TODO use timeout to detect disconnect
#define MC_RX_SMALL_TIMEOUT_MS 30   // TODO use timeout to detect disconnect
#define MC_PARSE_TIMEOUT       1500 // TODO: shorten based on 'ot' requested

// Controller settings
#define MC_RPM_LIMIT     (11)  // rpm x 1000
#define MC_CURRENT_LIMIT (70)  // Amps
#define MC_MAX_VOLTAGE   (340) // Volts
#define MC_MOT_TMP_LIMIT (95)  // Celsius
#define MC_CTL_TMP_LIMIT (70)  // Celsius
#define MC_POLE_PAIR_CT  (15)  // number of pole pairs (poles / 2)
#define MC_UPDATE_PERIOD (15)  // ms

typedef enum
{
    MC_LINK_DISCONNECTED,   // Waits until there are signs of life
    MC_LINK_ATTEMPT,        // Signs of life, attempts serial mode
    MC_LINK_VERIFYING,      // Searches for verification of serial mode
    MC_LINK_DELAY,          // Waits before connected
    MC_LINK_CONNECTED,      // In serial mode, thankfully
    MC_LINK_FAILED          // Failed to connect, retrying
} motor_link_state_t;

typedef enum
{
    MC_LINK_ERROR_NONE,
    MC_LINK_ERROR_NOT_SERIAL,
    MC_LINK_ERROR_CMD_TIMEOUT,
    MC_LINK_ERROR_GEN_TIMEOUT
} motor_link_error_t;

typedef enum
{
    MC_CONFIG_START
} motor_config_state_t;
typedef enum
{
    MC_DISCONNECTED, // Have not established connection yet
    MC_CONNECTED,    // Receiving messages as expected
    MC_CONFIG,       // Adjusting parameters in analog mode
    MC_ERROR         // :(
} motor_state_t;

typedef struct 
{
    // Motor status
    uint16_t      init_time;                        // Current init timing
    uint32_t      last_serial_time;                 // Last time verification of serial mode found
    uint32_t      last_parse_time;                  // Last time data was succesfully parsed
    motor_link_state_t link_state;                  // Current state of the USART connection
    uint8_t       config_step;                      // Step in sending config params via mcUpdateConfig
    motor_state_t motor_state;                      // Current motor state
    bool config_sent;                               // Config params have been sent
    motor_link_error_t last_link_error;             // Last link error
    const bool          *hv_present;                // Address to variable stating if the tractive system has voltage present to power MCs

    // Motor configuration
    bool          is_inverted;                      // Send 'f' versus 'r' for positive torque command
    uint32_t      rx_timeout;                       // Dynamically set timeout to determine connection status

    // Motor outputs
    uint16_t      voltage_x10;                      
    uint16_t      current_x10;
    uint16_t      curr_power_x10;                   // Last torque command percent output sent x10
    uint32_t      rpm;
    uint8_t       controller_temp;
    uint8_t       motor_temp;

    // Communications
    q_handle_t   *tx_queue;                         // FIFO for tx commands to be sent via DMA
    bool          data_stale;                       // True if data has not been parsed for MC_PARSE_TIMEOUT
    volatile usart_rx_buf_t *rx_buf;                // Circular buffer for receiving via DMA
} motor_t;

void mcInit(motor_t *m, bool is_inverted, q_handle_t *tx_queue, 
            usart_rx_buf_t *rx_buf, const bool *hv_present);
void mcSetPower(float power, motor_t *m);
void mcPeriodic(motor_t *m);

#endif