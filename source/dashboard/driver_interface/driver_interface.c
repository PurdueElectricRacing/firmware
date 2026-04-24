#include "can_library/faults_common.h"
#include "can_library/generated/DASHBOARD.h"
#include "common/freertos/freertos.h"
#include "common/heartbeat/heartbeat.h"
#include "common/phal/usart.h"

#include "driver_interface.h"
#include "lcd.h"
#include "main.h"
#include "strbuf.h"

#define ACTION_QUEUE_LENGTH 10
DEFINE_QUEUE(action_queue, interface_action_t, ACTION_QUEUE_LENGTH);
volatile uint16_t data_mark_index = 0;

void EXTI9_5_IRQHandler() {
    // EXTI9 (LEFT Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF9) {
        xQueueSendFromISR(action_queue, &(interface_action_t){BACK_PAGE}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF9;
    }

    // EXTI8 (RIGHT Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF8) {
        xQueueSendFromISR(action_queue, &(interface_action_t){FORWARD_PAGE}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF8;
    }

    // EXTI7 (DOWN Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF7) {
        xQueueSendFromISR(action_queue, &(interface_action_t){MENU_DOWN}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF7;
    }

    // EXTI6 (UP Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF6) {
        xQueueSendFromISR(action_queue, &(interface_action_t){MENU_UP}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF6;
    }

    // EXTI5 (MARK_DATA Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF5) {
        xQueueSendFromISR(action_queue, &(interface_action_t){MARK_DATA}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF5;
    }
}

void EXTI15_10_IRQHandler() {
    // EXTI15 (SELECT button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF15) {
        xQueueSendFromISR(action_queue, &(interface_action_t){SELECT_BUTTON}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF15;
    }

    // EXTI14 (START button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF14) {
        xQueueSendFromISR(action_queue, &(interface_action_t){START_BUTTON}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF14;
    }
}

void driver_interface_init() {
    INIT_QUEUE(action_queue, interface_action_t, ACTION_QUEUE_LENGTH);

    // Enable the SYSCFG clock for interrupts
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Map EXTI lines to correct GPIO ports
    // PA5, PC6, PC7 (EXTI 5,6, 7)
    SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI5_PA | SYSCFG_EXTICR2_EXTI6_PC | SYSCFG_EXTICR2_EXTI7_PC);
    // PC8, PC9 (EXTI8, 9)
    SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PC | SYSCFG_EXTICR3_EXTI9_PC);
    // PB14, PB15 (EXTI14, 15)
    SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI14_PB | SYSCFG_EXTICR4_EXTI15_PB);

    // Unmask interrupts (EXTI lines 5,6,7,8,9,14,15)
    EXTI->IMR1 |= (EXTI_IMR1_IM5 | EXTI_IMR1_IM6 | EXTI_IMR1_IM7 | EXTI_IMR1_IM8 | EXTI_IMR1_IM9 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15);

    // Falling edge trigger only (pull-up buttons)
    EXTI->RTSR1 &= ~(EXTI_RTSR1_RT5 | EXTI_RTSR1_RT6 | EXTI_RTSR1_RT7 | EXTI_RTSR1_RT8 | EXTI_RTSR1_RT9 | EXTI_RTSR1_RT14 | EXTI_RTSR1_RT15);

    EXTI->FTSR1 |= (EXTI_FTSR1_FT5 | EXTI_FTSR1_FT6 | EXTI_FTSR1_FT7 | EXTI_FTSR1_FT8 | EXTI_FTSR1_FT9 | EXTI_FTSR1_FT14 | EXTI_FTSR1_FT15);

    NVIC_SetPriority(EXTI9_5_IRQn, 7);
    NVIC_SetPriority(EXTI15_10_IRQn, 7);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void action_dispatcher(void) {
    interface_action_t action;
    while (xQueueReceive(action_queue, &action, 0) == pdTRUE) {
        switch (action) {
            case UPDATE_PAGE:
                updatePage();
                break;
            case MENU_UP:
                moveUp();
                break;
            case MENU_DOWN:
                moveDown();
                break;
            case BACK_PAGE:
                backPage();
                break;
            case FORWARD_PAGE:
                advancePage();
                break;
            case SELECT_BUTTON:
                selectItem();
                break;
            case START_BUTTON:
                // CAN_SEND_start_button(true);
                // todo: non-periodic start button using a callback
                break;
            case MARK_DATA:
                CAN_SEND_mark_data(xTaskGetTickCount(), data_mark_index);
                data_mark_index++;
                break;
            default:
                break;
        }
    }
}

extern status_leds_t status_leds;
void set_external_leds(void) {
    // dont update the external LEDS until we're out of preflight
    if (status_leds.state == HEARTBEAT_STATE_PREFLIGHT) {
        return;
    }

    bool precharge_incomplete = is_latched(FAULT_ID_PRECHARGE_INCOMPLETE);
    PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, !precharge_incomplete);

}

void driver_interface_periodic(void) {
    action_dispatcher(); // Process Pending Driver Actions

    updateTelemetryPages();

    LCD_tx_update(); // dump the command

    set_external_leds();
}