#include "plettenberg.h"

static void mc_set_param(uint8_t value, char *param, motor_t *m);
static void mcInitialize(motor_t* m);
static int16_t mc_parse(char *rx_buf, uint8_t start, char *search_term, uint32_t *val_addr);

void mc_init(motor_t *m, bool is_inverted, q_handle_t *tx_queue){
    *m = (motor_t) {
        .is_inverted = is_inverted,
        .tx_queue    = tx_queue,
        .data_stale  = true,
        .last_parse_time = 0xFFFF0000,
        .motor_state = MC_DISCONNECTED,
        .init_state = 0,
        .last_rx_time = 0xFFFF0000,
        .rx_timeout = MC_RX_LARGE_TIMEOUT_MS
    };

    return;
}

void mc_set_power(float power, motor_t *m)
{
    // don't even try if disconnected
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

    uint8_t absolute_ones   = (pow_x10 / 10) % 10;
    uint8_t absolute_tenths = pow_x10 % 10;

    if (pow_x10 <= 1) cmd[idx - 1] = '0';
    else
    {
        /* Absolute Command Mode */
        // tens (only if 10% or greater)
        if (pow_x10 >= 100) cmd[idx++] = (pow_x10 == 1000) ? MC_MAX_POWER : (pow_x10 / 100) + '0';
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
    cmd[idx++] = '\0';
    qSendToBack(m->tx_queue, cmd);
}

// Communicates initialization state through motor_state_t
static void mcInitialize(motor_t* m) {
    char     cmd[2];
    char     tmp_rx_buf[MC_MAX_RX_LENGTH];
    int8_t   search_idx;
    uint8_t  i;
    uint32_t throwaway;

    motor_init_t next_state;

    next_state = m->init_state;

    switch (m->init_state) {
        case MC_INIT_START:
        {
            m->init_time = 0;
            next_state = MC_INIT_WAITING;

            // Try to put the motor controller into serial mode
            cmd[0] = MC_SERIAL_MODE;
            cmd[1] = '\0';

            qSendToBack(m->tx_queue, cmd);
        }

        case MC_INIT_WAITING:
        {
            for (i = 0; i < MC_MAX_RX_LENGTH; ++i) {
                tmp_rx_buf[i] = m->rx_buf[i];
            }
            search_idx = mc_parse(tmp_rx_buf, 0, "S=", &throwaway);

            if (search_idx > 0) {
                next_state = MC_INIT_COMPLETE;
                cmd[0] = MC_SET_TIMEOUT;
                cmd[1] = '\0';

                qSendToBack(m->tx_queue, cmd);

                return;
            }

            if (m->init_time == (1250 / 15)) {
                next_state = MC_INIT_FAILED;
            }

            ++m->init_time;

            break;
        }

        case MC_INIT_FAILED:
        {
            // State exists merely for the breakpoint
            next_state = MC_INIT_START;

            break;
        }

        case MC_INIT_COMPLETE:
        {
            // Again, exists for the breakpoint
            asm("nop");
        }
    }

    m->init_state = next_state;
}

// Returns false if the motor is not good to spin
bool mc_periodic(motor_t* m) {
    char     tmp_rx_buf[MC_MAX_RX_LENGTH];
    int16_t  curr;
    uint8_t  i;
    uint32_t val_buf;
    
    // Can't be initialized if precharge isn't complete
    if (can_data.main_hb.precharge_state == 0) {
        m->motor_state = MC_DISCONNECTED;
        m->init_state = MC_INIT_START;

        return false;
    }

    // Check if we're in the middle of initialization
    if (m->motor_state == MC_INITIALIZING) {
        if (m->init_state == MC_INIT_COMPLETE) {
            m->motor_state = MC_CONNECTED;
        } else {
            mcInitialize(m);

            return false;
        }
    }

    // Check if we're disconnected and run initialization
    if (m->motor_state == MC_DISCONNECTED) {
        m->motor_state = MC_INITIALIZING;
        m->init_state = MC_INIT_START;

        mcInitialize(m);

        return false;
    }

    // 0        9        18       27       36       45         56        66
    // S=3.649V,a=0.000V,PWM= 787,U= 34.9V,I=  3.7A,RPM=  1482,con= 28°C,mot= 26°C

    // Copy buffer so it doesn't change underneath us
    for (i = 0; i < MC_MAX_RX_LENGTH; ++i) {
        tmp_rx_buf[i] = m->rx_buf[i];
    }

    // Parse voltage
    curr = mc_parse(tmp_rx_buf, curr, "U=", &val_buf);
    if (curr >= 0) m->voltage_x10 = (uint16_t) val_buf;

    // Parse current
    if (curr >= 0) curr = mc_parse(tmp_rx_buf, curr, "I=", &val_buf);
    if (curr >= 0) m->current_x10 = (uint16_t) val_buf;

    // Parse RPM
    if (curr >= 0) curr = mc_parse(tmp_rx_buf, curr, "RPM=", &val_buf);
    if (curr >= 0) m->rpm = val_buf;

    // Parse controller temp
    if (curr >= 0) curr = mc_parse(tmp_rx_buf, curr, "con=", &val_buf);
    if (curr >= 0) m->controller_temp = (uint8_t) val_buf;

    // Parse motor temp
    if (curr >= 0) curr = mc_parse(tmp_rx_buf, curr, "mot=", &val_buf);
    if (curr >= 0) m->motor_temp = (uint8_t) val_buf;

    // Update parse time
    if (curr >= 0) m->last_parse_time = sched.os_ticks;
    m->data_stale = (sched.os_ticks - m->last_parse_time > MC_PARSE_TIMEOUT);

    return true;
}

int16_t mc_parse(char *rx_buf, uint8_t start, char *search_term, uint32_t *val_addr)
{
    // start searching for search term at start, if value found return the index
    // the next location to look at, if not return -1
    // keeps adding as long as it sees leading spaces, multiplies by 10 if decimal point found
    uint8_t search_length = strlen(search_term);
    bool match = false;
    uint8_t curr = 0xFF;

    for (uint8_t i = start; i < MC_MAX_RX_LENGTH + start; ++i) 
    {
        if (rx_buf[i % MC_MAX_RX_LENGTH] == search_term[0])
        {
            match = true;
            // possible match, check entire term matches
            for (uint8_t j = 0; j < search_length; ++j)
            {
                if (rx_buf[(i + j) % MC_MAX_RX_LENGTH] != search_term[j])
                {
                    // not a match, continue searching
                    match = false;
                    break;
                }
            }
            if (match)
            {
                curr = i % MC_MAX_RX_LENGTH;
                break;
            }
        }
    }
    if (!match) return -1;
    // destroy match to prevent re-reading same data
    // rx_buf[curr] = '\0';
    curr = (curr + search_length) % MC_MAX_RX_LENGTH;

    uint32_t val = 0;

    /* Extract value */
    for (uint8_t i = curr; i < MC_MAX_RX_LENGTH + curr; ++i)
    {
        char c = rx_buf[i % MC_MAX_RX_LENGTH];
        if ((c == ' ' && val == 0) || c == '.') continue;
        else if (c >= '0' && c <= '9')
        {
            val = (val * 10) + (c - '0');
        }
        else
        {
            curr = i % MC_MAX_RX_LENGTH;
            break;
        }
    }

    *val_addr = val;
    return curr;
}
