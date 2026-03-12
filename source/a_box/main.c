/**
 * @file main.c
 * @brief "Abox" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
 */

#include "main.h"

#include <stdint.h>

#include "adbms.h"
#include "common/can_library/faults_common.h"
#include "common/can_library/generated/A_BOX.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"

SPI_InitConfig_t bms_spi_config = {
    .data_len      = 8,
    .nss_sw        = false, // BMS drive CS pin manually to ensure correct timing
    .nss_gpio_port = SPI1_CS_PORT,
    .nss_gpio_pin  = SPI1_CS_PIN,
    .rx_dma_cfg    = nullptr,
    .tx_dma_cfg    = nullptr,
    .periph        = SPI1,
    .cpol          = 0,
    .cpha          = 0,
    .data_rate     = 500'000, // 500 kHz SPI clock for ADBMS6380
};

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN1RX_PA11,
    GPIO_INIT_FDCAN1TX_PA12,

    // CCAN
    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13,

    // SPI for BMS
    GPIO_INIT_OUTPUT(SPI1_CS_PORT, SPI1_CS_PIN, GPIO_OUTPUT_ULTRA_SPEED),
    GPIO_INIT_SPI1SCK_PA5,
    GPIO_INIT_SPI1MISO_PA6,
    GPIO_INIT_SPI1MOSI_PA7,

    // ISENSE and VSENSE
    GPIO_INIT_ANALOG(ISENSE_GPIO_PORT, ISENSE_GPIO_PIN),
    GPIO_INIT_ANALOG(VBATT_GPIO_PORT, VBATT_GPIO_PIN),

    // Input status pins
    GPIO_INIT_INPUT(CHARGER_CONNECTED_PORT, CHARGER_CONNECTED_PIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(NOT_PRECHARGE_COMPLETE_PORT, NOT_PRECHARGE_COMPLETE_PIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(IMD_STATUS_PORT, IMD_STATUS_PIN, GPIO_INPUT_OPEN_DRAIN),

    // BMS SDC Control
    GPIO_INIT_OUTPUT(BMS_SDC_CTRL_PORT, BMS_SDC_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED)
};

static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;
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

adbms_bms_t g_bms                              = {0};
uint8_t g_bms_tx_buf[ADBMS_SPI_TX_BUFFER_SIZE] = {0};

static constexpr float MIN_V_FOR_BALANCE     = 3.0f;
static constexpr float MIN_DELTA_FOR_BALANCE = 0.1f;

extern void HardFault_Handler(void);
void bms_task(void);
void background_can_update(void);
void check_faults(void);
void report_telemetry(void);
void heartbeat_task(void);

DEFINE_TASK(bms_task, 200, osPriorityHigh, STACK_2048);
DEFINE_TASK(background_can_update, 2, osPriorityHigh, STACK_2048);
DEFINE_TASK(check_faults, 10, osPriorityNormal, STACK_512);
DEFINE_TASK(fault_library_periodic, A_BOX_FAULT_SYNC_PERIOD_MS, osPriorityNormal, STACK_1024);
DEFINE_TASK(report_telemetry, PACK_STATS_PERIOD_MS, osPriorityLow, STACK_512);
DEFINE_TASK(heartbeat_task, HEARTBEAT_PERIOD_MS, osPriorityLow, STACK_256);

int main(void) {
    // Hardware Initilization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // Set CS high to start
    adbms6380_set_cs_high(&bms_spi_config);

    if (!PHAL_SPI_init(&bms_spi_config)) {
        HardFault_Handler();
    }

    adbms_init(&g_bms, &bms_spi_config, g_bms_tx_buf);

    if (false == PHAL_FDCAN_init(FDCAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, CCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    NVIC_SetPriority(FDCAN1_IT0_IRQn, 5);
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 5);

    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    CAN_library_init();

    // Kernel initalization
    osKernelInitialize();

    START_TASK(bms_task);
    START_TASK(background_can_update);
    START_TASK(check_faults);
    START_TASK(fault_library_periodic);
    START_TASK(report_telemetry);
    START_TASK(heartbeat_task);

    osKernelStart(); // no way home

    return 0;
}

void report_telemetry() {
    // todo isense
    uint16_t pack_voltage_mv = (uint16_t)(g_bms.sum_voltage * 1000.0f);
    CAN_SEND_pack_stats(pack_voltage_mv, 0, g_bms.avg_therm_temp);
}

void background_can_update() {
    CAN_tx_update();
    CAN_rx_update();
}

void heartbeat_task() {
    // preflight animation for the first 1.5 seconds after boot
    if (OS_TICKS <= PREFLIGHT_ANIMATION_DURATION_MS) {
        static uint32_t sweep_index = 0;

        // Creates a sweeping pattern
        switch (sweep_index++ % 3) {
            case 0:
                PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 1);
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
                PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
                break;
            case 1:
                PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 0);
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
                PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
                break;
            case 2:
                PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 0);
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
                PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 1);
                break;
        }
        return;
    }

    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);

    bool can_stale = (OS_TICKS - last_can_rx_time_ms >= CONN_LED_TIMEOUT_MS);
    PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, can_stale);
}

void bms_task() {
    adbms_periodic(&g_bms, MIN_V_FOR_BALANCE, MIN_DELTA_FOR_BALANCE);
}

void check_faults() {
    // IMD
    // todo check polarity of this signal
    bool imd_status = PHAL_readGPIO(IMD_STATUS_PORT, IMD_STATUS_PIN);
    update_fault(FAULT_ID_IMD, imd_status);

    // Temperature related
    update_fault(FAULT_ID_PACK_OVERTEMP, g_bms.max_therm_temp);
    update_fault(FAULT_ID_PACK_WARM, g_bms.max_therm_temp);
    update_fault(FAULT_ID_PACK_COLD, g_bms.min_therm_temp);
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN); // Turn on error LED
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
