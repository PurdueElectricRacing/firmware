/*System Includes*/
#include "main.h"
#include <stdint.h>

#include "common/daq/can_parse_base.h"
#include "common/phal/can.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/queue/queue.h"
#include "source/front_driveline/can/can_parse.h"

/* Module Includes */
#include "can_parse.h"

GPIOInitConfig_t gpio_config[] = {
    // Shock Pots
    GPIO_INIT_ANALOG(SHOCK_POT_L_GPIO_Port, SHOCK_POT_L_Pin),
    GPIO_INIT_ANALOG(SHOCK_POT_R_GPIO_Port, SHOCK_POT_R_Pin),

    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,

    //Magnometer

    //Strain Gauge

    //IR Temperature

    //Load Cells

};

// Communication queues
q_handle_t q_tx_usart;

int main(void) {


    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    schedStart();

    return 0;
}

void can_worker_task() {
    // Process all received CAN messages
    while (qIsEmpty(&q_rx_can) == false) {
        canRxUpdate();
    }

    // Drain all CAN transmit queues
    canTxUpdate();
}

//TODO: make shock pot calcs cleaner spread values into variables 
/**
 * @brief Processes and sends shock potentiometer readings
 *
 * Converts raw ADC values from left and right shock potentiometers into parsed displacement values
 * and sends them through CAN bus. Values are scaled linearly and adjusted for droop.
 */
int16_t shock_l_displacement;
int16_t shock_r_displacement;

void sendShockpots() {
    uint16_t shock_l = raw_adc_values.shock_left;
    uint16_t shock_r = raw_adc_values.shock_right;

    //left calculation variables 

    //compute total voltage range
    float shock_l_range = (float) POT_VOLT_MIN_L - POT_VOLT_MAX_L;

    //get percentage 
    float shock_l_percent = shock_l / shock_l_range;

    //scale percentage to distance 
    float shock_l_scaled = shock_l_percent * POT_MAX_DIST;

    //adjust bc of droop 
    float shock_l_adjusted = POT_MAX_DIST - shock_l_scaled - POT_DIST_DROOP_L;

    //convert to int and switch sign
    shock_l_displacement = (int16_t) (-shock_l_adjusted);


    //right calculation vaiables
    float shock_r_range = (float) POT_VOLT_MIN_R - POT_VOLT_MAX_R;
    float shock_r_percent = shock_r / shock_r_range;
    float shock_r_scaled = shock_r_percent * POT_MAX_DIST;
    float shock_r_adjusted = POT_MAX_DIST -  shock_r_scaled - POT_DIST_DROOP_R;
    shock_r_displacement = (int16_t) (-shock_r_adjusted);


    SEND_SHOCK_FRONT_DRIVELINE(shock_l_displacement, shock_r_displacement);
}