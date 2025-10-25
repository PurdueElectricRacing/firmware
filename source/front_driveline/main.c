/*System Includes*/
#include "common/daq/can_parse_base.h"
#include "common/phal/can.h"


/* Module Includes */
#include "can_parse.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/queue/queue.h"
#include "source/dashboard/can/can_parse.h"
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    // Shock Pots
    GPIO_INIT_ANALOG(SHOCK_POT_L_GPIO_Port, SHOCK_POT_L_Pin),
    GPIO_INIT_ANALOG(SHOCK_POT_R_GPIO_Port, SHOCK_POT_R_Pin),

    //Magnometer

    //Strain Gauge

    //IR Temperature

    //Load Cells


    
};

void can_worker_task() {
    // Process all received CAN messages
    while (qIsEmpty(&q_rx_can) == false) {
        canRxUpdate();
    }

    // Drain all CAN transmit queues
    canTxUpdate();
}