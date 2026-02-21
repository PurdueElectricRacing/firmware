/**
 * @file main.c
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "main.h"

#include <stdint.h>

#include "common/can_library/faults_common.h"
#include "common/can_library/generated/MAIN_MODULE.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "amk/amk2.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
};

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
ClockRateConfig_t clock_config                  = {
                     .clock_source           = CLOCK_SOURCE_HSE,
                     .use_pll                = false,
                     .system_clock_target_hz = TargetCoreClockrateHz,
                     .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
                     .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
                     .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

AMK_t test_amk;
bool is_precharge_complete = false;

extern void HardFault_Handler(void);

int main(void) {
    // Hardware Initilization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (false == PHAL_FDCAN_init(FDCAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, MCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    AMK_init(&test_amk, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &is_precharge_complete);

    // Software Initalization
    osKernelInitialize();


    // no way home
    osKernelStart();

    return 0;
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = ERROR_LED_PIN;
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
