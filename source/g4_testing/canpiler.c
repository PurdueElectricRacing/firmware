#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_CANPILER)

#include <string.h>

#include "common/can_library/generated/G4_TESTING.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/psched/psched.h"
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13,
    GPIO_INIT_FDCAN3RX_PA8,
    GPIO_INIT_FDCAN3TX_PB4,
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

static void can_tx_100hz(void);

int main() {
    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // Send on CAN2, receive on CAN3
    if (!PHAL_FDCAN_init(FDCAN1, false, 500000U)) {
        HardFault_Handler();
    }

    CAN_library_init();

    // NVIC
    NVIC_SetPriority(FDCAN1_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);

    return 0;
}

static void can_tx_100hz(void) {
    // PHAL_FDCAN_testStandard();
    CAN_SEND_test_msg(123);
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_FDCAN
