#include "main.h"

#include <stdint.h>

#include "adbms.h"
#include "common/can_library/faults_common.h"
#include "common/can_library/generated/A_BOX.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"

dma_init_t spi_rx_dma_config    = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config    = SPI1_TXDMA_CONT_CONFIG(NULL, 1);
SPI_InitConfig_t bms_spi_config = {
    .data_len      = 8,
    .nss_sw        = false, // BMS drive CS pin manually to ensure correct timing
    .nss_gpio_port = SPI1_CS_PORT,
    .nss_gpio_pin  = SPI1_CS_PIN,
    .rx_dma_cfg    = &spi_rx_dma_config,
    .tx_dma_cfg    = &spi_tx_dma_config,
    .periph        = SPI1,
    .cpol          = 0,
    .cpha          = 0,
    .data_rate     = 1000000, // 1 MHz SPI clock for ADBMS6380
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
    // Hardware Initilization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (false == PHAL_FDCAN_init(FDCAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, CCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    if (!PHAL_SPI_init(&bms_spi_config)) {
        HardFault_Handler();
    }

    g_adbms.spi = &bms_spi_config;
    g_adbms.enable_balance = true;

    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    // Software Initalization
    osKernelInitialize();

    createThread(adbms_g_periodic);

    // no way home
    osKernelStart();

    return 0;
}


// int main(void) {
//     osKernelInitialize();

//     /* HAL Initilization */
//     if (0 != PHAL_configureClockRates(&clock_config)) {
//         HardFault_Handler();
//     }
//     if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
//         HardFault_Handler();
//     }
//     if (!PHAL_SPI_init(&bms_spi_config)) {
//         HardFault_Handler();
//     }
//     g_adbms.spi = &bms_spi_config;
//     g_adbms.enable_balance = true;

//     // // todo enable user button interrupt
//     // RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

//     createThread(adbms_g_periodic);

//     osKernelStart();

//     return 0;
// }

void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}
