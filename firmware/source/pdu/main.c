/**
 * @file main.c
 * @brief "PDU" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "main.h"

/* System Includes */
#include "can_library/faults_common.h"
#include "can_library/generated/PDU.h"
#include "common/rtos/rtos.h"
#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/pin_defs/g474ret6.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/utils/countof.h"
#include "common/watchdog/watchdog.h"

/* Module Includes */
#include "cooling_fsm.h"

PHAL_GPIO_InitConfig_t gpio_config[] = {
    // VCAN
    PHAL_PIN_DEFS_FDCAN1_TX_PA12,
    PHAL_PIN_DEFS_FDCAN1_RX_PA11,
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();

// Thread Defines
DEFINE_CAN_TASKS();
RTOS_DEFINE_TASK(cooling_fsm_periodic, COOLING_FSM_PERIOD_MS, TASK_PRIORITY_NORMAL, STACK_1024);
RTOS_DEFINE_TASK(fault_library_periodic, 100, TASK_PRIORITY_LOW, STACK_1024);
DEFINE_WATCHDOG_TASK();

int main() {
    // Hardware Initialization
    PHAL_RCC_init(PHAL_RCC_HSE_16MHZ);
    WDG_init();
    if (!PHAL_GPIO_init(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }
    
    PHAL_FDCAN_init(FDCAN1, VCAN_BAUD_RATE);

    CAN_init();

    START_CAN_TASKS();
    CAN_SEND_pdu_init(WDG_get_CSR());
    RTOS_START_TASK(cooling_fsm_periodic);
    RTOS_START_TASK(fault_library_periodic);
    START_WATCHDOG_TASK();

    vTaskStartScheduler();

    return 0;
}

void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL = 0;
    ERROR_LED_PORT->BSRR = (1U << ERROR_LED_PIN);
    while (1) {
        __asm__("NOP"); // spin
    }
}
