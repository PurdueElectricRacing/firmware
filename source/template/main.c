/* System Includes */
#include <math.h>

#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/psched/psched.h"
#include "stm32l432xx.h"

/* Module Includes */
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_OUTPUT_LOW_SPEED)};

ClockRateConfig_t clock_config = {
    .system_source             = SYSTEM_CLOCK_SRC_PLL,
    .system_clock_target_hz    = 80000000,
    .pll_src                   = PLL_SRC_HSI16,
    .vco_output_rate_target_hz = 160000000,
    .ahb_clock_target_hz       = 80000000,
    .apb1_clock_target_hz      = 80000000, // / 16,
    .apb2_clock_target_hz      = 80000000 / 16,
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void ledBlink(void);
void HardFault_Handler(void);

int main(void) {
    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    taskCreate(ledBlink, 500);

    schedStart();

    return 0;
}

void ledBlink() {
    PHAL_toggleGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

void HardFault_Handler() {
    PHAL_writeGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0);
    while (1) {
        __asm__("nop");
    }
}