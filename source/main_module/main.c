/**
 * @file main.c
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * 
 */

#include "main.h"

#include <stdint.h>

#include "common/can_library/generated/MAIN_MODULE.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/amk/amk.h"

/* PER HAL Initialization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // TSAL
    GPIO_INIT_OUTPUT(TSAL_RED_CTRL_PORT, TSAL_RED_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(TSAL_GREEN_CTRL_PORT, TSAL_GREEN_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(TSAL_RTM_ENABLE_PORT, TSAL_RTM_ENABLE_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(TSAL_FAULT_PORT, TSAL_FAULT_PIN, GPIO_INPUT_PULL_DOWN),

    // Brake and Buzzer
    GPIO_INIT_OUTPUT(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BUZZER_PORT, BUZZER_PIN, GPIO_OUTPUT_LOW_SPEED),

    // SDC
    GPIO_INIT_OUTPUT(ECU_SDC_CTRL_PORT, ECU_SDC_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(SDC_MUX_PORT, SDC_MUX_PIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(SDC_MUX_S3_PORT, SDC_MUX_S3_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_MUX_S2_PORT, SDC_MUX_S2_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_MUX_S1_PORT, SDC_MUX_S1_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_MUX_S0_PORT, SDC_MUX_S0_PIN, GPIO_OUTPUT_LOW_SPEED),

    // Input status pins
    GPIO_INIT_INPUT(BMS_STATUS_PORT, BMS_STATUS_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(VBATT_ECU_PORT, VBATT_ECU_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(VMC_ECU_PORT, VMC_ECU_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN, GPIO_INPUT_PULL_DOWN),

    // VCAN
    GPIO_INIT_FDCAN2TX_PB13,
    GPIO_INIT_FDCAN2RX_PB12,

    // MCAN
    GPIO_INIT_FDCAN3TX_PA15,
    GPIO_INIT_FDCAN3RX_PA8
};

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
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

extern void HardFault_Handler(void);

void ledblink() {
    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
}

void can_worker() {
    CAN_rx_update();
    CAN_tx_update();
}

defineThreadStack(ledblink, 500, osPriorityLow, 256);
defineThreadStack(can_worker, 15, osPriorityNormal, 2048);

int main(void) {
    // Hardware Initialization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN3, false, MCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN3_IT0_IRQn);
    CAN_library_init();

    // Software Initialization
    osKernelInitialize();

    createThread(ledblink);
    createThread(can_worker);

    // no way home
    osKernelStart();

    return 0;
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN);
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
