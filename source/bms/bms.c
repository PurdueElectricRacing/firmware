#include "bms.h"

// Static function prototypes
static uint8_t searchNextError(void);
static void memsetu(uint8_t* ptr, uint8_t val, uint32_t size);

bms_t   bms;
uint8_t error_ff;

// @funcname: bmsStatus
//
// @brief: Posts BMS status and tracks mode
void bmsStatus()
{
    uint8_t i;
    uint8_t e_flag;
    uint8_t current_ec;
    static uint8_t ec_ticks_rem;
    static uint8_t flag;
    static uint8_t ec_state;

    flag = !flag;
    e_flag = 0;
    
    // Setup the error light to blink error codes (rolling through all)
    switch (ec_state) {
        case 0:
        {
            current_ec = searchNextError();
            
            if (current_ec) {
                e_flag = 1;
                ec_state = 1;
                ec_ticks_rem = current_ec * 2 - (flag ? 1 : 0);
            }

            break;
        }

        case 1:
        {
            e_flag = 1;

            if (--ec_ticks_rem == 0) {
                ec_ticks_rem = 4;
                ec_state = 2;
            }

            break;
        }

        case 2:
        {
            if (--ec_ticks_rem == 0) {
                if (current_ec == E_CNT - 1) {
                    current_ec = -1;
                }

                ec_state = 0;
            }

            break;
        }
    }

    PHAL_writeGPIO(LED_HEART_GPIO_Port, LED_HEART_Pin, flag);
    PHAL_writeGPIO(LED_ERR_GPIO_Port, LED_ERR_Pin, e_flag ? flag : 0);
    PHAL_writeGPIO(LED_CONN_GPIO_Port, LED_CONN_Pin, bms.afe_con);

    if (bms.veh_con == 1)
    {
        bms.op_mode = MODE_DISCHARGE;
    }
    else
    {
        bms.op_mode = MODE_IDLE;
    }
}

// @funcname: initBMS
//
// @brief: Initializes BMS parameters and scheduler
void initBMS(SPI_InitConfig_t* hspi)
{
    memsetu((uint8_t*) &bms, 0, sizeof(bms));
    bms.spi = hspi;
    bms.temp_master = 0;
    bms.error |= error_ff;

#ifdef BMS_ACCUM
    bms.cell_count = 10;
    bms.temp_count = 20;
    bms.curr_sense_conn = 1;
#endif
#ifdef BMS_LV
    bms.cell_count = 8;
    bms.temp_count = 0;
    bms.curr_sense_conn = 0;
#endif

    checkConn();

    while ((bms.error & 0x1)) {
        PHAL_writeGPIO(LED_ERR_GPIO_Port, LED_ERR_Pin, 1);
        PHAL_writeGPIO(LED_HEART_GPIO_Port, LED_HEART_Pin, 0);
        checkConn();
    }
}

// @funcname: txCAN
//
// @brief: Send BMS data out
void txCAN(void)
{
    uint8_t i;
    static uint8_t state;

    switch (state)
    {
        case 0:
        {
            SEND_PACK_INFO(q_tx_can, bms.cells.mod_volts_raw, bms.error, bms.cells.balance_flags & ~bms.cells.balance_mask);
            SEND_CELL_INFO(q_tx_can, 0, bms.error & (1U << 1), bms.error & (1U << 2));
        }

        default:
        {
            for (i = 0; i < 4; i++)
            {
                SEND_VOLTS_CELLS(q_tx_can, i, bms.cells.chan_volts_raw[i * 3], bms.cells.chan_volts_raw[i * 3 + 1], bms.cells.chan_volts_raw[i * 3 + 2]);
            }

            for (i = 0; i < 4; i++)
            {
                SEND_TEMPS_CELLS(q_tx_can, i, bms.cells.chan_temps_conv[i * 3], bms.cells.chan_temps_conv[i * 3 + 1], bms.cells.chan_temps_conv[i * 3 + 2]);
            }

            SEND_POWER_LIM(q_tx_can, bms.p_lim.temp_max, 0);
        }
    }

    if (++state == 10)
    {
        state = 0;
    }
}

// @funcname: setPLim
//
// @brief: Determine the master power limit to use
void setPLim(void)
{
    uint16_t lim_temp = bms.p_lim.soc_max;

    if (bms.p_lim.temp_max < lim_temp)
    {
        lim_temp = bms.p_lim.temp_max;
    }

    if (bms.p_lim.v_max < lim_temp)
    {
        lim_temp = bms.p_lim.v_max;
    }

    bms.master_p_lim = lim_temp;
}

// @funcname: calcMisc
//
// @brief: Run miscellaneous, pack level calcs
void calcMisc(void)
{
    bms.power_out = bms.current_out * bms.voltage_out;
}

void checkLVStatus(void)
{
    static float mod_volts_last;

    if (bms.curr_sense_conn)
    {
        return;
    }

    if (bms.cells.mod_volts_raw > (mod_volts_last + CELL_CHARGE_IMPLAUS))
    {
        bms.current_out = -10;
    }
    else if (bms.cells.mod_volts_raw < (mod_volts_last - CELL_CHARGE_IMPLAUS))
    {
        bms.current_out = 10;
    }
    else
    {
        bms.current_out = 0;
    }

    mod_volts_last = bms.cells.mod_volts_raw;
}

// @funcname: checkSleep
//
// @brief: Checks if we want to sleep, and ensures that all
//         tasks are in a solid state for sleep
void checkSleep(void)
{
    // Wait for tasks to spool down (handled on a case by case basis)
    if (bms.no_sleep)
    {
        return;
    }

    // Handle WWDG
    IWDG->KR  = 0x5555;
    IWDG->PR  = 7;
    IWDG->RLR = 0x7ff;

    while ((IWDG->SR & 0b111) != 0);
    IWDG->KR  = 0xAAAA;

    // Put everything to sleep (AFE autosleeps after 2s)

    // Finalize sleep parameters

    // Night night
    schedPause();
}

// @funcname: canTxUpdate
//
// @brief: Send frames in queue
void canTxUpdate(void)
{
    CanMsgTypeDef_t tx_msg;
    static uint8_t ret;

    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        ret += PHAL_txCANMessage(&tx_msg);

        if (ret) {
            bms.error |= (1U << E_CAN_TX);
        } else {
            --ret;
            bms.error &= ~(1U << E_CAN_TX);
        }
    }
}

// @funcname: searchNextError
//
// @brief: Determines next error code to display
//
// @return: Next error code + 1
static uint8_t searchNextError(void)
{
    uint8_t i, ec_low, entry_ec, cnt;
    static uint8_t current_ec;

    ec_low = E_CNT;
    entry_ec = current_ec;
    cnt = 0;

    for (i = 1; i <= E_CNT; i++) {
        if (bms.error & (1U << (i - 1))) {
            ++cnt;

            if (i < current_ec && ec_low == E_CNT) {
                ec_low = i;
            } else if (i > current_ec) {
                current_ec = i;

                return current_ec;
            }
        }
    }

    if (ec_low < current_ec) {
        current_ec = ec_low;
    } else if (entry_ec == current_ec && ec_low != E_CNT) {
        current_ec = ec_low;
    }

    return cnt ? current_ec : 0;
}

// @funcname: memsetu
//
// @brief: Simple memset routine
//
// @param: ptr: Pointer to location to set
// @param: val: Value to set each memory address to
// @param: size: Length of data to set
static void memsetu(uint8_t* ptr, uint8_t val, uint32_t size)
{
    // Locals
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        ptr[i] = val;
    }
}