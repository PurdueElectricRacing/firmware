#include "plettenberg.h"

void mc_init(motor_t *m, bool is_inverted, q_handle_t *tx_queue){
    *m = (motor_t) {
        .is_inverted = is_inverted,
        .tx_queue = tx_queue
    };

    char cmd[MC_MAX_TX_LENGTH];
    cmd[0] = MC_SERIAL_MODE;
    cmd[1] = '\0';
    qSendToBack(m->tx_queue, cmd);
    return;
} /* mc_serial_init() */

void mc_set_power(float power, motor_t *m)
{
    // determine mode and clamp power from 0% - 100%
    char mode = (m->is_inverted) ? MC_REVERSE : MC_FORWARD;
    if (power < 0)
    {
        power *= -1;
        mode = MC_BREAK;
    }
    uint16_t pow_x10 = MIN((((uint16_t) power) * 10), 1000);

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

    if (incremen_ones + incremen_tenths < 1 + absolute_ones + absolute_tenths)
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
        for (absolute_tenths += idx; idx < absolute_ones; idx++) cmd[idx] = MC_INCREASE_TENTH;
    }
    // termination
    cmd[idx++] = '\0';
    qSendToBack(m->tx_queue, cmd);
    m->curr_power_x10 = pow_x10;
}

void mc_stop(motor_t *m){
    mc_set_power(0, m);
} /* mc_stop() */

/*
 *  Reads the data being sent from the motor controller
 */
void mc_parse(char* data, motor_t *m) {
    if (data[0] != 'S') return;
    
    // replaces spaces with '0'
    for (int i = 0; i < MC_MAX_RX_LENGTH; i++) if (data[i] == ' ') data[i] = '0';    
    
    // S=3.649V,a=0.000V,PWM= 787,U= 34.9V,I= 3.7A,RPM= 1482,con= 28°C,mot= 26°C

    m->voltage = ((data[30] - '0')* 100) + ((data[31] - '0')* 10) + (data[32] - '0') + ((data[34] - '0') / 10);
    //Should be >0 when the voltage is in the correct range(200 - 336), or <0 if otherwise
    m->proper_voltage = (m->voltage < (CELL_MAX_V * 80)) && (m->voltage > (CELL_MIN_V * 80));
    bool is_over_powered = false;

    m->phase_current = ((data[39] - '0') * 100) + ((data[40] - '0') * 10) + (data[41] - '0') + ((data[43] - '0') / 10);
    m->is_over_powered = (m->voltage * m->phase_current) > 60000; 

    float new_controller_temp = ((data[61] - '0') * 100) + ((data[62] - '0') * 10) + (data[63] - '0');
    if (!(m->controller_temp == 0)) {
        m->con_temp_slope = new_controller_temp - m->controller_temp;
    }
    m->controller_temp = new_controller_temp;

    float new_motor_temp = ((data[71] - '0')* 100) + ((data[72] - '0') * 10) + (data[73] - '0');
    if (!(m->motor_temp == 0)) {
        m->motor_temp_slope = new_motor_temp - m->motor_temp;
    }
    m->motor_temp = new_motor_temp;

    m->rpm = (data[50] - '0') * 100000 + (data[51] - '0') * 10000 + (data[52] - '0') * 1000 + 
             (data[53] - '0') * 100    + (data[54] - '0') * 10    + (data[55] - '0');

} /* read_motor_controller() */