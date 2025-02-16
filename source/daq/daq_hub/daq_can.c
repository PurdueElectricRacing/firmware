
#include <string.h>

#include "common/phal_F4_F7/can/can.h"
#include "common/freertos/freertos.h"
#include "daq_can.h"
#include "daq_hub.h"

// TODO move to freertos queues
can_stats_t can_stats[CAN_BUS_COUNT];
static uint32_t mbx_last_send_time[CAN_BUS_COUNT][CAN_TX_MAILBOX_CNT];

#define CAN_TX_BLOCK_TIMEOUT (30 * 16000) // clock rate 16MHz, 15ms * 16000 cyc / ms

static void initCANParseBase()
{
    memset(can_stats, 0, sizeof(can_stats) * sizeof(*can_stats));
}

void initCANParse()
{
    initCANParseBase();
    //initCANFilter(); // !!! NO CAN FILTER FOR DAQ !!!!!!!!!!!!!
}

void canTxSendToBack(CanMsgTypeDef_t *msg)
{
    uint32_t t = 0;
    /* Don't use multiple mailboxes to guarantee in-order transmission */
    while (!PHAL_txMailboxFree(CAN1, 0) && (t++ < CAN_TX_BLOCK_TIMEOUT));
    if (t < CAN_TX_BLOCK_TIMEOUT) PHAL_txCANMessage(msg, 0);
    else
    {
        can_stats[BUS_ID_CAN1].tx_of++;
        daq_catch_error();
    }
}

void canTxUpdate(void)
{
    // DAQ sends TX immediately to guarantee in-order transmission (bootloader)
    // So a separate TX update function is not needed
}
