/**
 * @file main.c
 * @brief "Torque Vector" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * 
 */

#include "main.h"

#include <stdint.h>

#include "common/can_library/generated/TORQUE_VECTOR.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"

/* PER HAL Initialization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN2TX_PB13,
    GPIO_INIT_FDCAN2RX_PB12,
};

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
ClockRateConfig_t clock_config = {
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


extern void HardFault_Handler(void);

void ledblink() {
    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
}

void can_worker_thread() {
    CAN_rx_update();
    CAN_tx_update();
}

defineThreadStack(ledblink, 500, osPriorityLow, 256);
defineThreadStack(can_worker_thread, 15, osPriorityNormal, 2048);

int main(void) {
    // Hardware Initialization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    CAN_library_init();

    // Software Initialization
    osKernelInitialize();

    createThread(ledblink);
    createThread(can_worker_thread);

    // no way home
    osKernelStart();

    return 0;
}

void torquevector_bl_cmd_CALLBACK(can_data_t* can_data) {
    (void)can_data;
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN);
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
