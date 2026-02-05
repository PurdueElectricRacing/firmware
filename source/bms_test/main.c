/* System Includes */
#include <math.h>
#include <stdint.h>

#include "common/phal/gpio.h"
#include "common/phal/spi.h"
#include "common/freertos/freertos.h"

/* Module Includes */
#include "main.h"
#include "adbms6830/adbms.h"
#include <string.h>
#include "adbms6830/commands.h"

dma_init_t spi_rx_dma_config = SPI2_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config = SPI2_TXDMA_CONT_CONFIG(NULL, 1);
SPI_InitConfig_t bms_spi_config = {
    .data_len  = 8,
    .nss_sw = false,
    .nss_gpio_port = SPI_CS_PORT,
    .nss_gpio_pin = SPI_CS_PIN,
    .rx_dma_cfg = &spi_rx_dma_config,
    .tx_dma_cfg = &spi_tx_dma_config,
    .periph = SPI2,
};

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(SPI_CS_PORT, SPI_CS_PIN, GPIO_OUTPUT_HIGH_SPEED), // PB12
    GPIO_INIT_AF(SPI_SCK_PORT, SPI_SCK_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN), // PB13
    GPIO_INIT_AF(SPI_MISO_PORT, SPI_MISO_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN), // PB14
    GPIO_INIT_AF(SPI_MOSI_PORT, SPI_MOSI_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN), // PB15
};


#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSI,
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

/* Function Prototypes */
void heartbeat_task();
void HardFault_Handler(void);

adbms_t g_adbms = {0};

extern void adbms_periodic(adbms_t *bms);

void adbms_g_periodic() {
	adbms_periodic(&g_adbms);
}

defineThreadStack(adbms_g_periodic, 200, osPriorityNormal, 2048);

int main(void) {
    osKernelInitialize();

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (!PHAL_SPI_init(&bms_spi_config)) {
        HardFault_Handler();
    }
    g_adbms.spi = &bms_spi_config;
    g_adbms.enable_balance = true;

    // // todo enable user button interrupt
    // RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    createThread(adbms_g_periodic);

    osKernelStart();

    return 0;
}

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}