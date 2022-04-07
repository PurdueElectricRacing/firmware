#include "plettenberg.h"

static void mc_set_startup_params(motor_t *m);

void mc_init(motor_t *m, bool is_inverted, q_handle_t *tx_queue, 
             volatile motor_rx_buf_t *rx_queue){
    *m = (motor_t) {
        .is_inverted = is_inverted,
        .tx_queue    = tx_queue,
        .rx_queue    = rx_queue,
        .motor_state = MC_DISCONNECTED
    };
    return;
}

void mc_set_power(float power, motor_t *m)
{
    if (m->motor_state != MC_CONNECTED) return;
    // determine mode and clamp power from 0% - 100%
    char mode = (m->is_inverted) ? MC_REVERSE : MC_FORWARD;
    if (power < 0)
    {
        power *= -1;
        mode = MC_BRAKE;
    }
    uint16_t pow_x10 = MIN(((uint16_t) (power * 10)), 1000);

    char cmd[MC_MAX_TX_LENGTH];
    uint8_t idx = 0;

    // mode
    cmd[idx++] = mode;

    // determine which is the shorter command: increment or absolute
    int16_t delta = pow_x10 - m->curr_power_x10;
    bool is_decrement = delta < 0;
    if (is_decrement) delta *= -1;

    uint8_t incremen_ones   = (delta / 10) % 10;
    uint8_t incremen_tenths = delta % 10;
    uint8_t absolute_ones   = (pow_x10 / 10) % 10;
    uint8_t absolute_tenths = pow_x10 % 10;

    if (pow_x10 <= 1) cmd[idx - 1] = '0';
    else if (delta < 100 && incremen_ones + incremen_tenths < 1 + absolute_ones + absolute_tenths)
    {
        /* Incremental Command Mode */
        // ones
        char c = is_decrement ? MC_DECREASE_ONE : MC_INCREASE_ONE;
        for (incremen_ones += idx; idx < incremen_ones; idx++) cmd[idx] = c;
        // tenths
        c = is_decrement ? MC_DECREASE_TENTH : MC_INCREASE_TENTH;
        for (incremen_tenths += idx; idx < incremen_tenths; idx++) cmd[idx] = c;
    }
    else
    {
        /* Absolute Command Mode */
        // tens
        cmd[idx++] = (pow_x10 == 1000) ? MC_MAX_POWER : (pow_x10 / 100) + '0';
        // ones
        for (absolute_ones += idx; idx < absolute_ones; idx++) cmd[idx] = MC_INCREASE_ONE;
        // tenths
        for (absolute_tenths += idx; idx < absolute_tenths; idx++) cmd[idx] = MC_INCREASE_TENTH;
    }
    // termination
    cmd[idx++] = '\0';
    qSendToBack(m->tx_queue, cmd);
    m->curr_power_x10 = pow_x10;
}

void mc_set_param(uint8_t value, char *param, motor_t *m)
{
    char cmd[MC_MAX_TX_LENGTH];
    uint8_t idx = 0;
    cmd[idx++] = MC_ENTER_ADJUST_MODE;
    cmd[idx++] = param[0];
    cmd[idx++] = param[1];
    cmd[idx++] = (value / 100) + '0';
    cmd[idx++] = ((value / 10) % 10) + '0';
    cmd[idx++] = (value % 10) + '0';
    cmd[idx++] = 'w';
    cmd[idx++] = 'p';
    //cmd[idx++] = MC_EXIT_ADJUST_MODE;
    cmd[idx++] = '\0';
    qSendToBack(m->tx_queue, cmd);
}

void mc_set_startup_params(motor_t *m)
{
    // serial mode
    char cmd[MC_MAX_TX_LENGTH];
    uint8_t i = 0;
    cmd[i++] = MC_SERIAL_MODE;
    // cmd[i++] = '\r';
    // cmd[i++] = '\n';
    cmd[i++] = '\0';
    qSendToBack(m->tx_queue, cmd);
    // configuration
    // mc_set_param(200, MC_PAR_UPDATE_PERIOD, m);
    // TODO: set tmp and curr limits higher, we will do the checking
    // mc_set_param(MC_CURRENT_LIMIT, MC_PAR_CURRENT_LIMIT, m);
    // mc_set_param(MC_MOT_TMP_LIMIT, MC_PAR_MOT_TMP_LIMIT, m);
    // mc_set_param(MC_CTL_TMP_LIMIT, MC_PAR_CTL_TMP_LIMIT, m);
}

/*
 *  Reads the data being sent from the motor controller
 */
bool mc_parse(motor_t *m) {
    // Check rx timeout
    if (m->motor_state == MC_CONNECTED &&
        sched.os_ticks - m->rx_queue->last_rx_time > MC_RX_TIMEOUT_MS)
        m->motor_state = MC_DISCONNECTED;

    // Check if actual data
    if (!m->rx_queue->free_has_data) return false;

    // Swap the free and read buffers
    m->rx_queue->free_has_data = false;
    char *data = m->rx_queue->free;
    m->rx_queue->free = m->rx_queue->read;
    m->rx_queue->read = data;

    /* Determine connection status */
    if (data[0] == 'T' || data[0] == 't')
    {
        // not in serial, set params
        mc_set_startup_params(m);
        return false;
    }
    else if (data[0] == 'S')
    {
        m->motor_state = MC_CONNECTED;
    }
    else
    {
        __asm__("nop"); // invalid data
        return false;
    }

    // replaces spaces with '0'
    for (int i = 0; i < MC_MAX_RX_LENGTH; i++) if (data[i] == ' ') data[i] = '0';    
    
    // 0        9        18       27       36       45         56        66
    // S=3.649V,a=0.000V,PWM= 787,U= 34.9V,I=  3.7A,RPM=  1482,con= 28°C,mot= 26°C
    if (data[27] != 'U') return false;
    m->voltage = ((data[29] - '0')* 100) + ((data[30] - '0')* 10) + (data[31] - '0') + ((data[33] - '0') / 10.0);
    //Should be >0 when the voltage is in the correct range(200 - 336), or <0 if otherwise
    m->proper_voltage = (m->voltage < (CELL_MAX_V * 80)) && (m->voltage > (CELL_MIN_V * 80));
    bool is_over_powered = false;

    if (data[36] != 'I') return false;
    m->phase_current = ((data[38] - '0') * 100) + ((data[39] - '0') * 10) + (data[40] - '0') + ((data[42] - '0') / 10.0);
    m->is_over_powered = (m->voltage * m->phase_current) > 60000; 

    if (data[45] != 'R') return false;
    m->rpm = (data[49] - '0') * 100000 + (data[50] - '0') * 10000 + (data[51] - '0') * 1000 + 
             (data[52] - '0') * 100    + (data[53] - '0') * 10    + (data[54] - '0');

    if (data[56] != 'c') return false;
    float new_controller_temp = ((data[60] - '0') * 100) + ((data[61] - '0') * 10) + (data[62] - '0');
    if (!(m->controller_temp == 0)) {
        m->con_temp_slope = new_controller_temp - m->controller_temp;
    }
    m->controller_temp = new_controller_temp;

    if (data[66] != 'm') return false;
    float new_motor_temp = ((data[70] - '0')* 100) + ((data[71] - '0') * 10) + (data[72] - '0');
    if (!(m->motor_temp == 0)) {
        m->motor_temp_slope = new_motor_temp - m->motor_temp;
    }
    m->motor_temp = new_motor_temp;

    return true;
}