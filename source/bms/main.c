/* System Includes */
#include "stm32l432xx.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/spi/spi.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"
#include "common/queue/queue.h"
#include <math.h>

/* Module Includes */
#include "main.h"
#include "bms.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(LED_CONN_GPIO_Port, LED_CONN_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(LED_HEART_GPIO_Port, LED_HEART_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(LED_ERR_GPIO_Port, LED_ERR_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT(CSB_AFE_GPIO_Port, CSB_AFE_Pin, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,
    GPIO_INIT_I2C1_SCL_PB6,
    GPIO_INIT_I2C1_SDA_PB7,
    GPIO_INIT_SPI1_SCK_PB3,
    GPIO_INIT_SPI1_MISO_PB4,
    GPIO_INIT_SPI1_MOSI_PA7,
};

dma_init_t spi_rx_dma_config = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config = SPI1_TXDMA_CONT_CONFIG(NULL, 1);

SPI_InitConfig_t spi_config = {
    .data_rate = 112500,
    .data_len = 8,
    .nss_sw = false,
    .nss_gpio_port = CSB_AFE_GPIO_Port,
    .nss_gpio_pin = CSB_AFE_Pin,
    .rx_dma_cfg = &spi_rx_dma_config,
    .tx_dma_cfg = &spi_tx_dma_config
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void HardFault_Handler(void);

q_handle_t q_tx_can;
q_handle_t q_rx_can;

int main(void) {
    // Data Structure Init
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    /* Start of Preflight Inspection */
    // HAL Init
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initCAN(CAN1, false))
    {
        HardFault_Handler();
    }
    if (!PHAL_SPI_init(&spi_config))
    {
        HardFault_Handler();
    }
#if EEPROM_ENABLED
    if(!PHAL_initI2C())
    {
        HardFault_Handler();
    }
#endif

    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    // Signify start of init
    PHAL_writeGPIO(LED_ERR_GPIO_Port, LED_ERR_Pin, 1);
    initBMS(&spi_config);

    /* End of Preflight Inspection */
    // Task Creation
    schedInit(SystemCoreClock);
    taskCreate(bmsStatus, 500);
    taskCreate(afeTask, 100);
    taskCreate(checkConn, 1000);
    schedStart();

    // Fire up the scheduler, and don't look back
    PHAL_writeGPIO(LED_ERR_GPIO_Port, LED_ERR_Pin, 0);
    schedStart();

    // If the scheduler returns somehow, some way, wait for watchdog reset
    HardFault_Handler();

    return 0;
}

void HardFault_Handler(void) {
    // Sit and wait for the watchdog to pull us out
    while (1);
}