/* System Includes */
#include "common/bootloader/bootloader_common.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/faults/faults.h"
#include "common/common_defs/common_defs.h"

/* Module Includes */
#include "main.h"
#include "source/torque_vector/can/can_parse.h"

GPIOInitConfig_t gpio_config[] = {
    // Status Indicators
    GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),

    // CAN
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source               =CLOCK_SOURCE_HSI,
    .use_pll                    =false,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

// #define TargetCoreClockrateHz 96000000
// ClockRateConfig_t clock_config = {
//     .clock_source               =CLOCK_SOURCE_HSI,
//     .use_pll                    =true,
//     .pll_src                    =PLL_SRC_HSI16,
//     .vco_output_rate_target_hz  =192000000,
//     .system_clock_target_hz     =TargetCoreClockrateHz,
//     .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
//     .apb1_clock_target_hz       =(TargetCoreClockrateHz / 4),
//     .apb2_clock_target_hz       =(TargetCoreClockrateHz / 4),
// };

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void heartBeatLED(void);
void preflightAnimation(void);
void preflightChecks(void);
extern void HardFault_Handler(void);

int main(void)
{
    /* Data Struct Initialization */

    /* HAL Initialization */
    if (0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }

    /* GPIO initialization */
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    configureAnim(preflightAnimation, preflightChecks, 74, 1000);

    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);

    taskCreate(heartBeatLED, 500);

    /* No Way Home */
    schedStart();

    return 0;
}

void preflightChecks(void)
{
    static uint16_t state;

    switch (state++)
    {
    case 300:
        if (!PHAL_initCAN(CAN1, false, VCAN_BPS))
        {
            HardFault_Handler();
        }
        NVIC_EnableIRQ(CAN1_RX0_IRQn);
        break;
    case 400:
        initFaultLibrary(FAULT_NODE_NAME, &q_tx_can[CAN1_IDX][CAN_MAILBOX_HIGH_PRIO], ID_FAULT_SYNC_TORQUE_VECTOR);
        break;
    default:
        if (state > 750)
        {
            initCANParse();
            registerPreflightComplete(1);
            state = 750; // prevent wrap around
        }
        break;
    }
}

void preflightAnimation(void)
{
    static uint32_t time;

    PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 0);
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 0);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);

    switch (time++ % 6)
    {
    case 0:
    case 5:
        PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 1);
        break;
    case 1:
    case 2:
    case 3:
        PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
        break;
    case 4:
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
        break;
    }
}

void heartBeatLED(void)
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);

    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
}

/* CAN Message Handling */
void CAN1_RX0_IRQHandler()
{
    canParseIRQHandler(CAN1);
}

void torquevector_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.torquevector_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while (1)
    {
        __asm__("nop");
    }
}
