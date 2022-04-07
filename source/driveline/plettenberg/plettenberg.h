#ifndef __PLETTENBERG_H__
#define __PLETTENBERG_H__

#include "common/phal_L4/usart/usart.h"
#include "common/common_defs/common_defs.h"
#include "common/psched/psched.h"
#include "common/queue/queue.h"
#include "string.h"
#include "stm32l432xx.h"

#define MC_MAX_TX_LENGTH 1500
#define MC_MAX_RX_LENGTH (77 + MC_MAX_TX_LENGTH)

#define MC_PREAMBLE_TIME 4000
#define MC_TIMEOUT_CONSTRAINT_TIME (1000 + MC_PREAMBLE_TIME)

#define MC_RX_LARGE_TIMEOUT_MS 1500
#define MC_RX_SMALL_TIMEOUT_MS 30

#define MC_SERIAL_MODE    's'
#define MC_ANALOG_MODE    'p'
#define MC_FORWARD        'f'
#define MC_REVERSE        'r'
#define MC_BRAKE          'b'
#define MC_MAX_POWER      'm'
#define MC_INCREASE_ONE   '+'
#define MC_DECREASE_ONE   '-'
#define MC_INCREASE_TENTH 'g'
#define MC_DECREASE_TENTH 'l'

#define MC_ENTER_ADJUST_MODE 'a'
#define MC_EXIT_ADJUST_MODE  'e'
#define MC_WRITE_PARAMS      "wp"

#define MC_PAR_CURRENT_LIMIT "cl"
#define MC_PAR_MOT_TMP_LIMIT "mt"
#define MC_PAR_CTL_TMP_LIMIT "ct"
#define MC_PAR_UPDATE_PERIOD "ot"

#define MC_CURRENT_LIMIT (100)
#define MC_MOT_TMP_LIMIT (100)
#define MC_CTL_TMP_LIMIT (80)

// Motor Controller Constants:
#define CELL_MAX_V 4.2 //May be increased to 4.25 in the future
#define CELL_MIN_V 2.5

typedef struct {
    uint32_t timeout;
    uint32_t last_rx_time;
    uint32_t init_time;
    char rx_buf[MC_MAX_RX_LENGTH];
} motor_rx_buf_t;

typedef enum
{
    MC_DISCONNECTED,
    MC_INITIALIZING,
    MC_CONNECTED,
    MC_ERROR    
} motor_state_t;

typedef struct 
{
    bool is_inverted;
    uint16_t curr_power_x10;
    q_handle_t *tx_queue;
    motor_rx_buf_t *rx_queue;
    motor_state_t motor_state;
    /* Values Read */
    float voltage;
    bool proper_voltage;
    float phase_current;
    bool is_over_powered;
    float controller_temp;
    float motor_temp;
    //These are integers, and the period of time per measurement is 1 second
    int motor_temp_slope;
    int con_temp_slope;
    uint32_t rpm;
} motor_t;

/**
 * @brief Initializes the motor in serial mode
 */
void mc_init(motor_t *m, bool is_inverted, q_handle_t *tx_queue, 
             volatile motor_rx_buf_t *rx_queue);
/**
 * @brief Positive power commands motor to move
 *        Negative power commands motor to break
 */
void mc_set_param(uint8_t value, char *param, motor_t *m);
void mc_set_power(float power, motor_t *m);
/**
 * @brief Reads the data being sent from the motor controller
 * @return returns false if the data was not succesfully parsed
 */
bool mc_parse(motor_t *m);

#endif