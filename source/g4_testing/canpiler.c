#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_CANPILER)

#include <string.h>

#include "common/can_library/generated/A_BOX.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/freertos/freertos.h"
#include "main.h"
#include "common/can_library/faults_common.h"

GPIOInitConfig_t gpio_config[] = {
    // GPIO_INIT_FDCAN2RX_PB12,
    // GPIO_INIT_FDCAN2TX_PB13
    GPIO_INIT_FDCAN1RX_PA11,
    GPIO_INIT_FDCAN1TX_PA12,

};

#define TargetCoreClockrateHz 16'000'000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 16'000'000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz      = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz      = (TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();

// void send_periodic() {
//     CAN_SEND_ccan_test(0x3);
// }

void send_periodic() {
    CAN_SEND_abox_version(GIT_HASH);
}

DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048);
DEFINE_TASK(send_periodic, 10, osPriorityNormal, 1024);

int main() {
    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (!PHAL_FDCAN_init(FDCAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    CAN_library_init();

    // NVIC
    NVIC_SetPriority(FDCAN1_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);

    osKernelInitialize();

    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(send_periodic);

    osKernelStart();

    return 0;
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_CANPILER
