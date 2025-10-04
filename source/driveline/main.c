/*System Includes*/
#include "common/phal/can.h"


/* Module Includes */
#include "can_parse.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    // Shock Pots
    GPIO_INIT_ANALOG(SHOCK_POT_L_GPIO_Port, SHOCK_POT_L_Pin),
    GPIO_INIT_ANALOG(SHOCK_POT_R_GPIO_Port, SHOCK_POT_R_Pin),
};