#include "f4_testing.h"
#if (F4_TESTING_CHOSEN == TEST_CANPILER)

#include "common/can_library/generated/PDU.h"
#include "common/phal/adc.h"
#include "common/phal/can.h"
#include "common/phal/dma.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/freertos/freertos.h"

GPIOInitConfig_t gpio_config[] = {
    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1
};

#define TargetCoreClockrateHz 16'000'000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 160'000'000,
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
    CAN_SEND_pdu_version(GIT_HASH);
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

    if (!PHAL_initCAN(CAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    
    CAN_library_init();

    // NVIC
    NVIC_SetPriority(CAN1_RX0_IRQn, 6);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    osKernelInitialize();

    START_TASK(send_periodic);
    START_TASK(can_worker);
    osKernelStart();

    return 0;
}

// just to avoid linker error
void cooling_driver_request_CALLBACK(can_data_t* can_data) {
    (void)can_data; // unused
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // F4_TESTING_CHOSEN == TEST_CANPILER
