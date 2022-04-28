#include "plettenberg.h"

static void mc_set_startup_params(motor_t *m);
static void mc_set_param(uint8_t value, char *param, motor_t *m);
static int16_t mc_parse(char *rx_buf, uint8_t start, char *search_term, uint32_t *val_addr);

void mc_init(motor_t *m, bool is_inverted, q_handle_t *tx_queue){
    *m = (motor_t) {
        .is_inverted = is_inverted,
        .tx_queue    = tx_queue,
        .data_valid  = false,
        .motor_state = MC_DISCONNECTED,
        .last_rx_time = 15000,
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
    cmd[i++] = '\0';
    qSendToBack(m->tx_queue, cmd);
    // TODO: set tmp and curr limits higher, we will do the checking
}

int16_t mc_parse(char *rx_buf, uint8_t start, char *search_term, uint32_t *val_addr)
{
    // start searching for search term at start, if value found return the index
    // the next location to look at, if not return -1
    // keeps adding as long as it sees leading spaces, multiplies by 10 if decimal point found
    uint8_t search_length = strlen(search_term);
    bool match = true;
    uint8_t curr = 0xFF;

    for (uint8_t i = start; i < MC_MAX_RX_LENGTH + start; ++i) 
    {
        if (rx_buf[i % MC_MAX_RX_LENGTH] == search_term[0])
        {
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

static char tmp_rx_buf[MC_MAX_RX_LENGTH] = {'\0'};
bool mc_periodic(motor_t *m) {
    // Check rx timeout
    bool timed_out = (sched.os_ticks - m->last_rx_time > m->rx_timeout);

    /* Update connection status */
    if (timed_out)
    {
        m->motor_state = MC_DISCONNECTED;
        m->rx_timeout = MC_RX_LARGE_TIMEOUT_MS;
        m->data_valid = false;
    }
    else if (m->motor_state == MC_DISCONNECTED)
    {
        m->boot_start_time = sched.os_ticks;
        m->motor_state = MC_INITIALIZING;
    }
    else if (m->motor_state == MC_INITIALIZING &&
            sched.os_ticks - m->boot_start_time > MC_BOOT_TIME)
    {
        mc_set_startup_params(m);
        m->motor_state = MC_CONNECTED;
    }

    // Switch from large to small timeout after timeout constraint time
    if (m->motor_state == MC_CONNECTED          && 
        m->rx_timeout == MC_RX_LARGE_TIMEOUT_MS &&
        sched.os_ticks - m->boot_start_time > MC_TIMEOUT_CONSTRAINT_TIME)
    {
        m->rx_timeout = MC_RX_SMALL_TIMEOUT_MS;
    }

    // 0        9        18       27       36       45         56        66
    // S=3.649V,a=0.000V,PWM= 787,U= 34.9V,I=  3.7A,RPM=  1482,con= 28°C,mot= 26°C
    int16_t curr = 0;
    uint32_t val_buf = 0;

    // freeze state of dma buffer
    for (uint8_t i = 0; i < MC_MAX_RX_LENGTH; ++i) tmp_rx_buf[i] = m->rx_buf[i];

    /* Voltage */
    curr = mc_parse(tmp_rx_buf, curr, "U=", &val_buf);
    if (curr < 0) return false;
    m->voltage_x10 = (uint16_t) val_buf;

    /* Current */
    curr = mc_parse(tmp_rx_buf, curr, "I=", &val_buf);
    if (curr < 0) return false;
    m->current_x10 = (uint16_t) val_buf;

    /* RPM */
    curr = mc_parse(tmp_rx_buf, curr, "RPM=", &val_buf);
    if (curr < 0) return false;
    m->rpm = val_buf;

    /* Controller temp */
    curr = mc_parse(tmp_rx_buf, curr, "con=", &val_buf);
    if (curr < 0) return false;
    m->controller_temp = (uint8_t) val_buf;

    /* Motor temp */
    curr = mc_parse(tmp_rx_buf, curr, "mot=", &val_buf);
    if (curr < 0) return false;
    m->controller_temp = (uint8_t) val_buf;

    m->data_valid = true;

    return true;
}