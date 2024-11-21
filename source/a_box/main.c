/* System Includes */
#include "stm32f407xx.h"
#include "can_parse.h"
#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/can/can.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/dma/dma.h"

/* Module Includes */
#include "main.h"
#include "daq.h"
#include "orion.h"
#include "tmu.h"

#include "common/faults/faults.h"

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // TMU MUX
    // SPI
    
    // CLK
    GPIO_INIT_AF(GPIOA, 5, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    
    // MISO
    GPIO_INIT_AF(GPIOA, 6, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    
    // MOSI 
    GPIO_INIT_AF(GPIOA, 7, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    
    // Assuming U14 is populated (TMU B)
    GPIO_INIT_OUTPUT(GPIOC, 2, GPIO_OUTPUT_HIGH_SPEED),

    // LEDs

    // ERR
    GPIO_INIT_OUTPUT(GPIOE, 15, GPIO_OUTPUT_LOW_SPEED),

    // Heartbeat
    GPIO_INIT_OUTPUT(GPIOE, 13, GPIO_OUTPUT_LOW_SPEED),

    // Conn
    GPIO_INIT_OUTPUT(GPIOE, 14, GPIO_OUTPUT_LOW_SPEED),
};


#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

SPI_InitConfig_t spi_config = {
    .data_rate = TargetCoreClockrateHz / 64,
    .data_len = 8,
    .nss_sw = true,
    .nss_gpio_port = GPIOC,
    .nss_gpio_pin = 2,
    .rx_dma_cfg = NULL,
    .tx_dma_cfg = NULL,
    .periph = SPI1};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

extern void HardFault_Handler();

void preflightChecks();
void preflightAnimation();
void read_max_chip_id();

int main (void)
{
    /* Data Struct init */

    /* HAL Initilization */
    PHAL_trimHSI(HSI_TRIM_A_BOX);
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (1 != PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    /* Module init */
    schedInit(APB1ClockRateHz * 2); // See Datasheet DS11451 Figure. 4 for clock tree

    /* Task Creation */
    schedInit(SystemCoreClock);
    configureAnim(preflightAnimation, preflightChecks, 75, 750);

    taskCreate(read_max_chip_id, 1000);

    /* No Way Home */
    schedStart();

    return 0;
}

// *** Startup configuration ***
void preflightChecks(void)
{
   static uint16_t state;
    uint8_t charger_speed_def = 0;

    switch (state++)
    {
    case 0 :
        break;
    case 1:
        if (!PHAL_SPI_init(&spi_config))
        {
            HardFault_Handler();
        }
        spi_config.data_rate = APB2ClockRateHz / 16;

        // Raise chip select line
        PHAL_writeGPIO(GPIOC, 2, 1);
        break;
    case 700:
        break;
    default:
        if (state > 750)
        {
            registerPreflightComplete(1);
            state = 751;
        }
        break;
    }
}

void preflightAnimation(void)
{
   static uint32_t time = 0;

   PHAL_writeGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, 0);
   PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
   PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 0);

   switch (time++ % 3)
   {
       case 0:
           PHAL_writeGPIO(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin, 1);
           break;
       case 1:
           PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
           break;
       case 2:
           PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
           break;
   }
}

void a_box_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.a_box_bl_cmd.cmd == BLCMD_RST) {
        Bootloader_ResetForFirmwareDownload();
    }
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERROR_LED_GPIO_Port, ERROR_LED_Pin, 1);
    while (1)
    {
        __asm__("nop");
    }
}

void read_max_chip_id(void)
{
    static uint8_t spi_rx_buff[3] = {0};
    static uint8_t spi_tx_buff[3] = {0};

    while (PHAL_SPI_busy(&spi_config))
        ;

    #define TX_LEN (1)
    #define RX_LEN (2)

    /* Datasheet max22531 */
    /* https://www.analog.com/media/en/technical-documentation/data-sheets/max22530-max22532.pdf */
    /* Page 9 for read format */

    /* ID_BLK register is 0x00 16 bits */
    /* Page 27 */
    PHAL_SPI_transfer_noDMA(&spi_config, spi_tx_buff, TX_LEN, RX_LEN, spi_rx_buff);

    while (1)
    {
        // We will manually check what is returned here
        // Anticipating 129 or 0x81
        // Based on loose docs: https://github.com/analogdevicesinc/PyTrinamicMicro/blob/315849f30d3063c57b9ab54912ba8a99e683114a/PyTrinamicMicro/platforms/motionpy2/modules/max/max22531.py#L228
        __asm__("nop");
    }
}
