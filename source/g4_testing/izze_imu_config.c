#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == IZZE_IMU_CONFIG)

#include "can_library/generated/G4_TESTING.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/freertos/freertos.h"
#include "common/izze_imu/izze_imu.h"
#include "common/utils/countof.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOB)
#define HEARTBEAT_LED_PIN   (4)
#define ERROR_LED_PORT      (GPIOA)
#define ERROR_LED_PIN       (15)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (3)

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
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

static constexpr uint16_t NEW_CAN_BASE_ID = 0x4EE;
static_assert(NEW_CAN_BASE_ID >= 1, "CAN Base ID must be greater than or equal to 1");
static_assert(NEW_CAN_BASE_ID <= 0x7FF, "CAN Base ID must be less than or equal to 0x7FF");
// todo more assertions?

static constexpr uint32_t IMU_CONFIG_TIME_MS = 12'000; // "at least 10 seconds"
void config_imu() {
    if (OS_TICKS >= IMU_CONFIG_TIME_MS) {
        // set LED
        PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
        osThreadExit();
    }

    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
    CAN_SEND_IZZE_IMU_config(
        IZZE_IMU_PROGRAMMING_CONSTANT,
        NEW_CAN_BASE_ID,
        GYRO_SCALE_250DPS, ACCEL_SCALE_2G, ODR_104HZ, BIT_RATE_1MBPS
    );
}

DEFINE_CAN_TASKS();
DEFINE_TASK(config_imu, IZZE_IMU_CONFIG_PERIOD_MS, osPriorityNormal, 1024);

int main() {
    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }

    PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 0);
    PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
    PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);

    if (!PHAL_FDCAN_init(FDCAN2, false, GCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    CAN_init();

    osKernelInitialize();

    START_CAN_TASKS();
    START_TASK(config_imu);

    osKernelStart();

    return 0;
}

void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN);
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}


#endif // G4_TESTING_CHOSEN == IZZE_IMU_CONFIG
