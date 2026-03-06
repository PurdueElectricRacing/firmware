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
    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 16000000,
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


void can_worker() {
    CAN_rx_update();
    CAN_tx_update();
}

void send_periodic() {
    CAN_SEND_ccan_test(0x3);
}

DEFINE_TASK(send_periodic, 10, osPriorityNormal, 1024);
DEFINE_TASK(can_worker, 0, osPriorityLow, 1024);

int main() {
    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (!PHAL_FDCAN_init(FDCAN2, false, CCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    CAN_library_init();

    // NVIC
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    osKernelInitialize();

    START_TASK(send_periodic);
    START_TASK(can_worker);
    osKernelStart();

    return 0;
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_CANPILER
