/**
 * @file main.c
 * @brief "Abox" node source code
 * 
 * @author Sebastian Arthur (arthur31@purdue.edu), Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
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

// dma_init_t spi_rx_dma_config    = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
// dma_init_t spi_tx_dma_config    = SPI1_TXDMA_CONT_CONFIG(NULL, 1);
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
    .data_rate     = 500000, // 500 kHz SPI clock for ADBMS6380
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

    // Status Inputs + Analog Reads
    GPIO_INIT_INPUT(NOT_PRECHARGE_COMPLETE_PORT, NOT_PRECHARGE_COMPLETE_PIN, GPIO_INPUT_OPEN_DRAIN);
    GPIO_INIT_INPUT(IMD_STATUS_PORT, IMD_STATUS_PORT, GPIO_INPUT_PULL_DOWN);
    GPIO_INIT_INPUT(CHARGER_CONNECT_PORT, CHARGER_CONNECT_PIN, GPIO_INPUT_PULL_DOWN);

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

adbms_bms_t g_bms                              = {0};
uint8_t g_bms_tx_buf[ADBMS_SPI_TX_BUFFER_SIZE] = {0};

static constexpr float MIN_V_FOR_BALANCE     = 3.4f;
static constexpr float MIN_DELTA_FOR_BALANCE = 0.1f;

extern void HardFault_Handler(void);
void g_bms_periodic(void);

defineThreadStack(g_bms_periodic, 200, osPriorityHigh, 2048);

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

    if (false == PHAL_FDCAN_init(FDCAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, CCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    if (!PHAL_SPI_init(&bms_spi_config)) {
        HardFault_Handler();
    }

    adbms_init(&g_bms, &bms_spi_config, g_bms_tx_buf);

    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    // Software Initalization
    osKernelInitialize();

    createThread(g_bms_periodic);

    // no way home
    osKernelStart();

    return 0;
}

void g_bms_periodic() {
    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
    adbms_periodic(&g_bms, MIN_V_FOR_BALANCE, MIN_DELTA_FOR_BALANCE);

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
