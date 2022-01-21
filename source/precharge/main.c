/* System Includes */
#include "stm32l496xx.h"
#include "system_stm32l4xx.h"
#include "can_parse.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/quadspi/quadspi.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/rcc/rcc.h"


/* Module Includes */


/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,
    GPIO_INIT_CAN2RX_PB12,
    GPIO_INIT_CAN2TX_PB13,
    // QuadSPI Chip Selects
    GPIO_INIT_OUTPUT(QUADSPI_CS_FLASH_GPIO_Port, QUADSPI_CS_FLASH_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(QUADSPI_CS_FPGA_GPIO_Port, QUADSPI_CS_FPGA_Pin, GPIO_OUTPUT_LOW_SPEED),
    // QuadSPI Data/CLK
    GPIO_INIT_AF(QUADSPI_CLK_GPIO_Port, QUADSPI_CLK_Pin, 10, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(QUADSPI_IO0_GPIO_Port, QUADSPI_IO0_Pin, 10, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(QUADSPI_IO1_GPIO_Port, QUADSPI_IO1_Pin, 10, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(QUADSPI_IO2_GPIO_Port, QUADSPI_IO2_Pin, 10, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(QUADSPI_IO3_GPIO_Port, QUADSPI_IO3_Pin, 10, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    // I2C Bus
    GPIO_INIT_AF(I2C_SCL_GPIO_Port, I2C_SCL_Pin, 4, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(I2C_SDA_GPIO_Port, I2C_SDA_Pin, 4, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(I2C_WRITE_CONTROL_GPIO_Port, I2C_WRITE_CONTROL_Pin, GPIO_OUTPUT_LOW_SPEED),
    // Status LEDs
    GPIO_INIT_OUTPUT(ERROR_LED_GPIO_Port, ERROR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, GPIO_OUTPUT_LOW_SPEED)
};

ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_PLL,
    .system_clock_target_hz     =80000000,
    .pll_src                    =PLL_SRC_HSI16,
    .vco_output_rate_target_hz  =160000000,
    .ahb_clock_target_hz        =80000000,
    .apb1_clock_target_hz       =80000000 / 16,
    .apb2_clock_target_hz       =80000000 / 16,
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void canReceiveTest();
void canSendTest();
void Error_Handler();
void SysTick_Handler();
void canTxUpdate();
void blinkTask();
void PHAL_FaltHandler();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;



int main (void)
{
    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    /* HAL Initilization */
    if (0 != PHAL_configureClockRates(&clock_config))
        PHAL_FaltHandler();

    if (1 != PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
        PHAL_FaltHandler();
        
    if (1 != PHAL_initCAN(CAN1, false))
        PHAL_FaltHandler();

    if (1 != PHAL_qspiInit())
        PHAL_FaltHandler();
    
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    /* Module init */
    bitstreamInit();
    schedInit(APB1ClockRateHz * 2); // See Datasheet DS11451 Figure. 4 for clock tree
    initCANParse(&q_rx_can);

    /* Task Creation */
    schedInit(SystemCoreClock);
    taskCreate(canRxUpdate, RX_UPDATE_PERIOD);
    taskCreate(canTxUpdate, 5);
    taskCreate(bitstream10Hz, 100);
    taskCreate(bitstream100Hz, 10);
    schedStart();

    return 0;
}