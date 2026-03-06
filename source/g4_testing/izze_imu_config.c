#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == IZZE_IMU_CONFIG)

#include "common/can_library/generated/G4_TESTING.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/freertos/freertos.h"
#include "common/izze_imu/izze_imu.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13
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

void can_worker() {
    CAN_rx_update();
    CAN_tx_update();
}

static constexpr uint16_t NEW_CAN_BASE_ID = 0x4EE;
static_assert(NEW_CAN_BASE_ID >= 1, "CAN Base ID must be greater than or equal to 1");
static_assert(NEW_CAN_BASE_ID <= 0x7FF, "CAN Base ID must be less than or equal to 0x7FF");
// todo more assertions?

void send_periodic() {
    CAN_SEND_IZZE_IMU_config(
        IZZE_IMU_PROGRAMMING_CONSTANT,
        NEW_CAN_BASE_ID,
        GYRO_SCALE_250DPS, ACCEL_SCALE_2G, ODR_104HZ, BIT_RATE_1MBPS
    );
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

    if (!PHAL_FDCAN_init(FDCAN2, false, GCAN_BAUD_RATE)) {
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

#endif // G4_TESTING_CHOSEN == IZZE_IMU_CONFIG
