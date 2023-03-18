#include "plettenberg.h"

// Local defines

// 1 Byte Commands
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
#define MC_SET_TIMEOUT    't'
#define MC_ENTER_ADJUST_MODE 'a'
#define MC_EXIT_ADJUST_MODE  'e'

// 2 Byte Commands (Adjust mode only)
#define MC_SET_DEFAULT       "sd"
#define MC_WRITE_PARAMS      "wp"

// 5 Byte Commands (Adjust mode only)
#define MC_PAR_RPM_LIMIT           "rp"
#define MC_PAR_CURRENT_LIMIT       "cl"
#define MC_PAR_INPUT_CURRENT_LIMIT "il"
#define MC_PAR_MAX_VOLTAGE_LIMIT   "ov"
#define MC_PAR_MOT_TMP_LIMIT       "mt"
#define MC_PAR_CTL_TMP_LIMIT       "ct"
#define MC_PAR_POLE_PAIR_CT        "pp"
#define MC_PAR_UPDATE_PERIOD       "ot"

static char* mc_param_cmds[] = {MC_PAR_RPM_LIMIT, MC_PAR_CURRENT_LIMIT, MC_PAR_INPUT_CURRENT_LIMIT, 
                                MC_PAR_MAX_VOLTAGE_LIMIT, MC_PAR_MOT_TMP_LIMIT, MC_PAR_CTL_TMP_LIMIT,
                                MC_PAR_POLE_PAIR_CT, MC_PAR_UPDATE_PERIOD};
static uint16_t mc_param_vals[] = {MC_RPM_LIMIT, MC_CURRENT_LIMIT, MC_CURRENT_LIMIT, 
                                   MC_MAX_VOLTAGE, MC_MOT_TMP_LIMIT, MC_CTL_TMP_LIMIT,
                                   MC_POLE_PAIR_CT, MC_UPDATE_PERIOD};

// Local prototypes
static void mcSetParam(char *param, uint16_t value, motor_t *m);
static void mcSendTwoByteCmd(char *param, motor_t *m);
static void mcSendOneByteCmd(char command, motor_t *m);
static uint8_t mcCheckLinkState(motor_t* m);
static uint8_t mcUpdateConfig(motor_t* m);
static void mcParseMessage(motor_t *m);
static int16_t mcParseTerm(char *rx_buf, uint8_t start, char *search_term, uint32_t *val_addr);


/**
 * @brief Initializes the motor structure
 *        call before using any other functions
 * 
 * @param m           Motor controller
 * @param is_inverted Set true if the motor is oriented backwards
 * @param tx_queue    The USART transmit queue, assumes periodically serviced
 */
void mcInit(motor_t *m, bool is_inverted, q_handle_t *tx_queue, 
            usart_rx_buf_t *rx_buf, const bool *hv_present) {
    *m = (motor_t) {
        .is_inverted = is_inverted,
        .tx_queue    = tx_queue,
        .data_stale  = true,
        .last_parse_time = 0xFFFF0000,
        .motor_state = MC_DISCONNECTED,
        .last_serial_time = 0xFFF0000,
        .rx_buf      = rx_buf,
        .hv_present  = hv_present
    };
}


/**
 * @brief Sets the motor power if connected
 * 
 * @param power 100.0 is max output, -100.0 max regen
 * @param m     Motor controller
 */
void mcSetPower(float power, motor_t *m)
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

    uint8_t carry = 0;
    uint8_t tens_to_command = pow_x10 / 100;
    int8_t ones_to_command = (pow_x10 / 10) % 10;
    int8_t tenths_to_command = pow_x10 % 10;
    char c;

    // TODO: incremental command mode, but ensure there are bits
    // of absolute mode involved

    if (pow_x10 <= 1) cmd[idx - 1] = '0'; // replaces f, r, or b
    else
    {
        /* Absolute Command Mode */
        // Determine the number of symbols required
        // tenths
        carry = tenths_to_command > 5;
        if (carry) tenths_to_command -= 10;
        // ones
        ones_to_command += carry;
        carry = ones_to_command > 5 || tens_to_command == 0; // enforce decrement if less than 10% (ensures absolute mode)
        if (carry) ones_to_command -= 10;
        // tens
        tens_to_command += carry;

        // Populate symbols
        // tens
        cmd[idx++] = (tens_to_command == 10) ? MC_MAX_POWER : tens_to_command + '0';
        // ones
        if (ones_to_command < 0) 
        {
            c = MC_DECREASE_ONE;
            ones_to_command *= -1;
        }
        else c = MC_INCREASE_ONE;
        for (ones_to_command += idx; idx < ones_to_command; idx++) cmd[idx] = c;
        // tenths
        if (tenths_to_command < 0)
        {
            c = MC_DECREASE_TENTH;
            tenths_to_command *= -1;
        }
        else c = MC_INCREASE_TENTH;
        for (tenths_to_command += idx; idx < tenths_to_command; idx++) cmd[idx] = c;
    }
    // termination
    cmd[idx++] = '\0';
    qSendToBack(m->tx_queue, cmd);
    m->curr_power_x10 = pow_x10;
}


/**
 * @brief Sets a 5 byte adjust mode parameter
 *        Must first call mcSetParamStart
 *        To save, call mcSetParamEnd
 * 
 * @param value Value to set param to (<999)
 * @param param TWO character param
 * @param m     Motor controller
 */
static void mcSetParam(char *param, uint16_t value, motor_t *m)
{
    char cmd[6]; // 5 byte + '\0'
    if (value > 999 /* || // TODO: MC not ADJUST MODE */) return;
    cmd[0] = param[0];
    cmd[1] = param[1];
    cmd[2] = (value / 100) + '0';
    cmd[3] = ((value / 10) % 10) + '0';
    cmd[4] = (value % 10) + '0';
    cmd[5] = '\0';
    qSendToBack(m->tx_queue, cmd);
}


/**
 * @brief Sends a two byte command to the MC
 * 
 * @param param TWO character param
 * @param m Motor controller
 */
static void mcSendTwoByteCmd(char *param, motor_t *m)
{
    char cmd[3];
    cmd[0] = param[0];
    cmd[1] = param[1];
    cmd[2] = '\0';
    qSendToBack(m->tx_queue, cmd);
}

/**
 * @brief Sends a one byte command to the MC
 * 
 * @param command Command character
 * @param m Motor controller
 */
static void mcSendOneByteCmd(char command, motor_t *m)
{
    char cmd[2];
    cmd[0] = command;
    cmd[1] = '\0';
    qSendToBack(m->tx_queue, cmd);
}


/**
 * @brief Updates motor connection state and parses
 *        periodic status message
 * 
 * @param m      The motor controller to update
 */
void mcPeriodic(motor_t* m) 
{
    // Parse if recent data and update timing
    if (sched.os_ticks - m->rx_buf->last_msg_time < 3 * MC_LOOP_DT / 2)
        mcParseMessage(m);
    m->data_stale = (sched.os_ticks - m->last_parse_time > MC_PARSE_TIMEOUT);

    if (!mcCheckLinkState(m))
    {
        // in case of false disconnect, stop rotation
        if (m->motor_state != MC_DISCONNECTED) mcSetPower(0.0, m);
        m->motor_state = MC_DISCONNECTED;
        m->config_step = 0;
        return; // Don't even try other stuff if disconnected
    }
    if (!m->config_sent)
    {
        m->motor_state = MC_CONFIG;
        mcUpdateConfig(m);
    }
}


/**
 * @brief Updates motor link state based on
 *        last received message times
 * 
 * @param m      The motor controller to update
 */
static uint8_t mcCheckLinkState(motor_t* m) 
{
    char     tmp_rx_buf[MC_MAX_RX_LENGTH];
    int8_t   search_idx;
    uint32_t throwaway;
    uint8_t i;

    // check for disconnect
    if (sched.os_ticks - m->rx_buf->last_rx_time > MC_RX_LARGE_TIMEOUT_MS ||
        !*m->hv_present)
    {
        m->last_link_error = MC_LINK_ERROR_GEN_TIMEOUT;
        m->link_state = MC_LINK_DISCONNECTED;
        m->config_sent = false;
        return 0;
    }

    switch (m->link_state)
    {
        case MC_LINK_DISCONNECTED:
            if (sched.os_ticks - m->rx_buf->last_rx_time < 2 * MC_LOOP_DT)
            {
                m->link_state = MC_LINK_ATTEMPT;      // Message recently rx'd
            }
            break;
        case MC_LINK_ATTEMPT:
            m->init_time = 0;
            m->link_state = MC_LINK_VERIFYING;
            mcSendOneByteCmd(MC_SERIAL_MODE, m);
            break;
        case MC_LINK_VERIFYING:
            for (i = 0; i < MC_MAX_RX_LENGTH; ++i)      // Copy buffer to prevent from changing
                tmp_rx_buf[i] = m->rx_buf->rx_buf[i];
            search_idx = mcParseTerm(tmp_rx_buf, 0, "S=", &throwaway);
            if (search_idx != -1 && *m->hv_present)
            {
                m->link_state = MC_LINK_DELAY;
                m->init_time = 0;
            }
            else if (m->init_time == (1250 / MC_LOOP_DT))
            {
                m->link_state = MC_LINK_FAILED;
            }

            ++m->init_time;
            break;
        case MC_LINK_DELAY:
            if (m->init_time == (500 / MC_LOOP_DT))
            {
                m->link_state = MC_LINK_CONNECTED;
                m->motor_state = MC_CONNECTED;
                m->init_time = 0;
            }
            ++m->init_time;
            break;
        case MC_LINK_CONNECTED:
            if (m->motor_state == MC_CONFIG) 
            {
                if (m->config_sent &&
                    sched.os_ticks - m->rx_buf->last_msg_time < 2 * MC_LOOP_DT)
                    m->motor_state = MC_CONNECTED;
            }
            else
            {
                // COMMENTED OUT BECAUSE
                // The motor controller connection state will only be based on 
                // if any data is received, not necessarily a full message
                // if (sched.os_ticks - m->last_msg_time < MC_RX_LARGE_TIMEOUT_MS)
                // {
                //     if (sched.os_ticks - m->last_serial_time > MC_RX_LARGE_TIMEOUT_MS)
                //     {
                //         // probably not in serial mode any more
                //         m->last_link_error = MC_LINK_ERROR_NOT_SERIAL;
                //         m->link_state = MC_LINK_ATTEMPT;
                //     }
                // }
                // else
                // {
                //     // no msg received when it should have been
                //     m->last_link_error = MC_LINK_ERROR_CMD_TIMEOUT;
                //     m->link_state = MC_LINK_FAILED;
                // }
            }
            break;
        case MC_LINK_FAILED:
            asm("nop");     // exists for breakpoint
            m->link_state = MC_LINK_DISCONNECTED;
            break;
    }

    return m->link_state == MC_LINK_CONNECTED;
}


/**
 * @brief Send motor configuration params via
 *        adjust mode
 * 
 * @param m      The motor controller to update
 */
static uint8_t mcUpdateConfig(motor_t* m) 
{
    uint8_t i;

    i = sizeof(mc_param_cmds) / sizeof(*mc_param_cmds);

    if (m->config_step == 0)
    {
        m->config_sent = false;
        // mcSendOneByteCmd(MC_ENTER_ADJUST_MODE, m);
    }
    /* Parameters now set manually and written to non-volatile memory
    else if (m->config_step < i + 1)
    {
        mcSetParam(mc_param_cmds[m->config_step - 1],
                   mc_param_vals[m->config_step - 1], m);
    }
    else if (m->config_step < i + 3)
    {
        // Ensure exit :)
        mcSendOneByteCmd(MC_EXIT_ADJUST_MODE, m);
    }*/
    else if (m->config_step < 3)
    {
        mcSendOneByteCmd(MC_SET_TIMEOUT, m);
    }
    else
    {
        // wait another iteration to ensure exit command sent
        // TODO: verify comment
        // the link state checker will change the state
        // from adjust to serial once S= is detected
        m->config_step = 0;
        m->config_sent = true;
        return 1;
    }

    ++m->config_step;
    return 0;
}


/**
 * @brief Parses information from a message
 *        and verifies serial mode
 * 
 * @param m The motor controller to update
 */
static void mcParseMessage(motor_t *m)
{
    char     tmp_rx_buf[MC_MAX_RX_LENGTH];
    int16_t  curr;
    uint8_t  i;
    uint32_t val_buf;

    curr = m->rx_buf->last_msg_loc;

    // Copy buffer so it doesn't change underneath us
    for (i = 0; i < MC_MAX_RX_LENGTH; ++i) {
        tmp_rx_buf[i] = m->rx_buf->rx_buf[i];
    }

    // Parse the rx buffer, which may contain a message in the following format
    // 0        9        18       27       36       45         56        66
    // S=3.649V,a=0.000V,PWM= 787,U= 34.9V,I=  3.7A,RPM=  1482,con= 28°C,mot= 26°C

    // Check state
    curr = mcParseTerm(tmp_rx_buf, curr, "S=", &val_buf);
    if (curr >= 0) 
    {
        m->last_serial_time = sched.os_ticks;
        // if error is fixed without power cycle, recover
        if (m->motor_state == MC_ERROR) 
        {
            m->config_step = 0;
            m->motor_state = MC_CONFIG;
        }
    }
    else
    {
        // S not found, could be error message
        curr = m->rx_buf->last_msg_loc;
        curr = mcParseTerm(tmp_rx_buf, curr, "Err", &val_buf);
        if (curr >= 0)
        {
            // Error state
            if (m->motor_state != MC_ERROR)
            {
                mcSetPower(0.0, m);
                m->motor_state = MC_ERROR;
            }
            // TODO: parse error
        }
    }

    // Parse voltage
    if (curr >= 0) curr = mcParseTerm(tmp_rx_buf, curr, "U=", &val_buf);
    if (curr >= 0) m->voltage_x10 = (uint16_t) val_buf;

    // Parse current
    if (curr >= 0) curr = mcParseTerm(tmp_rx_buf, curr, "I=", &val_buf);
    if (curr >= 0) m->current_x10 = (uint16_t) val_buf;

    // Parse RPM
    if (curr >= 0) curr = mcParseTerm(tmp_rx_buf, curr, "RPM=", &val_buf);
    if (curr >= 0) m->rpm = val_buf;

    // Parse controller temp
    if (curr >= 0) curr = mcParseTerm(tmp_rx_buf, curr, "con=", &val_buf);
    if (curr >= 0) m->controller_temp = (uint8_t) val_buf;

    // Parse motor temp
    if (curr >= 0) curr = mcParseTerm(tmp_rx_buf, curr, "mot=", &val_buf);
    if (curr >= 0) m->motor_temp = (uint8_t) val_buf;

    // Update parse time
    if (curr >= 0) m->last_parse_time = sched.os_ticks;
}


/**
 * @brief Parses a number after the specified search term
 *        Starts searching at start and stops at start - 1 (wrap around)
 *        Multiplies the value by 10 if a decimal point is found
 * 
 * @param rx_buf      Buffer possibly containing a status message
 * @param start       Index to start searching for the search term 
 * @param search_term The term to search for in the buffer
 * @param val_addr    The address to store the parsed value
 * @return int16_t    Index of the next start location, -1 on failure
 */
int16_t mcParseTerm(char *rx_buf, uint8_t start, char *search_term, uint32_t *val_addr)
{
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
