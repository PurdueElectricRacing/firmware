#ifndef ADBMS_H
#define ADBMS_H

#include <stdint.h>
#include "common/phal/spi.h"


#define NUM_CELLS 16
#define ADBMS_WAKE_DELAY_MS 5

#define TX_CMD_LEN 4
#define TX_DATA_LEN 6
#define RX_DATA_LEN 8

#define CELL_GROUPS 3

typedef enum {
    ADBMS_IDLE = 0,
    ADBMS_CONNECTING,
    ADBMS_DISCHARGING,
    ADBMS_CHARGING
} adbms_state_t;

typedef enum {
    PWM_0_0_PCT = 0,      /* 0.0%  (default) */
    PWM_6_6_PCT,          /* 6.6%            */
    PWM_13_2_PCT,         /* 13.2%           */
    PWM_19_8_PCT,         /* 19.8%           */
    PWM_26_4_PCT,         /* 26.4%           */
    PWM_33_0_PCT,         /* 33.0%           */
    PWM_39_6_PCT,         /* 39.6%           */
    PWM_46_2_PCT,         /* 46.2%           */
    PWM_52_8_PCT,         /* 52.8%           */
    PWM_59_4_PCT,         /* 59.4%           */
    PWM_66_0_PCT,         /* 66.0%           */
    PWM_72_6_PCT,         /* 72.6%           */
    PWM_79_2_PCT,         /* 79.2%           */
    PWM_85_8_PCT,         /* 85.8%           */
    PWM_92_4_PCT,         /* 92.4%           */
    PWM_100_0_PCT,        /* ~100.0%         */
} adbms_pwm_duty_t;

typedef struct {
    SPI_InitConfig_t *spi;

    adbms_state_t curr_state;
    adbms_state_t next_state;

    uint8_t tx_buffer[TX_DATA_LEN];
    uint8_t rx_buffer[RX_DATA_LEN];

    uint32_t last_connection_time_ms;
    bool enable_balance;

    float cell_voltages[NUM_CELLS];
    adbms_pwm_duty_t cell_pwms[NUM_CELLS];

    uint32_t last_fault_time[27];
} adbms_t;

typedef struct {
    uint8_t CMD[2]; // CMD0 and CMD1
    uint16_t PEC15;
} adbms_cmd_pkt_t;

typedef struct {
    uint8_t DATA[TX_DATA_LEN];
    uint16_t DPEC10;
} adbms_tx_data_t;

typedef struct {
    uint8_t DATA[RX_DATA_LEN];
    uint16_t DPEC10;
} adbms_rx_data_t;

#define ADBMS_CONNECT_RETRY_PERIOD_MS 500

void adbms_wake(adbms_t *bms);
bool adbms_send_command(adbms_t *bms, const uint8_t cmd[2]);
bool adbms_send_data(adbms_t *bms, const uint8_t data[TX_DATA_LEN]);
bool adbms_connect(adbms_t *bms);
bool adbms_read_cell_voltages(adbms_t *bms);
bool adbms_passive_balance(adbms_t *bms);

#endif
