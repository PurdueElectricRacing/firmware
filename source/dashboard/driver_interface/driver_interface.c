#include "can_library/faults_common.h"
#include "can_library/generated/DASHBOARD.h"
#include "common/freertos/freertos.h"
#include "common/heartbeat/heartbeat.h"
#include "common/phal/usart.h"
#include "common/freertos/freertos.h"
#include "common/utils/clamp.h"

#include "driver_interface.h"
#include "lcd.h"
#include "main.h"
#include "strbuf.h"

#define ACTION_QUEUE_LENGTH 10
DEFINE_QUEUE(action_queue, interface_action_t, ACTION_QUEUE_LENGTH);
volatile uint16_t data_mark_index = 0;

static constexpr uint32_t INTERRUPT_DEBOUNCE_MS = 1;

void EXTI0_IRQHandler() {
    static volatile uint32_t last_interrupt_time = 0;
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_interrupt_time <= INTERRUPT_DEBOUNCE_MS) {
        EXTI->PR1 = EXTI_PR1_PIF0;
        return;
    }
    last_interrupt_time = now;

    if (EXTI->PR1 & EXTI_PR1_PIF0) {
        xQueueSendFromISR(action_queue, &(interface_action_t){EBB_MINUS}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF0;
    }
}

void EXTI1_IRQHandler() {
    static volatile uint32_t last_interrupt_time = 0;
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_interrupt_time <= INTERRUPT_DEBOUNCE_MS) {
        EXTI->PR1 = EXTI_PR1_PIF1;
        return;
    }
    last_interrupt_time = now;

    if (EXTI->PR1 & EXTI_PR1_PIF1) {
        xQueueSendFromISR(action_queue, &(interface_action_t){EBB_PLUS}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF1;
    }
}

void EXTI4_IRQHandler() {
    static volatile uint32_t last_interrupt_time = 0;
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_interrupt_time <= INTERRUPT_DEBOUNCE_MS) {
        EXTI->PR1 = EXTI_PR1_PIF4;
        return;
    }
    last_interrupt_time = now;

    if (EXTI->PR1 & EXTI_PR1_PIF4) {
        xQueueSendFromISR(action_queue, &(interface_action_t){TOGGLE_REGEN}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF4;
    }
}

void EXTI9_5_IRQHandler() {
    static volatile uint32_t last_interrupt_time = 0;
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_interrupt_time <= INTERRUPT_DEBOUNCE_MS) {
        EXTI->PR1 = EXTI_PR1_PIF5 | EXTI_PR1_PIF6 | EXTI_PR1_PIF7 |
                    EXTI_PR1_PIF8 | EXTI_PR1_PIF9;
        return;
    }
    last_interrupt_time = now;

    if (EXTI->PR1 & EXTI_PR1_PIF5) {
        xQueueSendFromISR(action_queue, &(interface_action_t){MARK_DATA}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF5;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF6) {
        xQueueSendFromISR(action_queue, &(interface_action_t){MENU_UP}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF6;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF7) {
        xQueueSendFromISR(action_queue, &(interface_action_t){MENU_DOWN}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF7;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF8) {
        xQueueSendFromISR(action_queue, &(interface_action_t){FORWARD_PAGE}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF8;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF9) {
        xQueueSendFromISR(action_queue, &(interface_action_t){BACK_PAGE}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF9;
    }
}

void EXTI15_10_IRQHandler() {
    static volatile uint32_t last_interrupt_time = 0;
    uint32_t now = xTaskGetTickCountFromISR();

    if (now - last_interrupt_time <= INTERRUPT_DEBOUNCE_MS) {
        EXTI->PR1 = EXTI_PR1_PIF11 | EXTI_PR1_PIF13 |
                    EXTI_PR1_PIF14 | EXTI_PR1_PIF15;
        return;
    }
    last_interrupt_time = now;

    if (EXTI->PR1 & EXTI_PR1_PIF11) {
        xQueueSendFromISR(action_queue, &(interface_action_t){TV1_PLUS}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF11;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF13) {
        xQueueSendFromISR(action_queue, &(interface_action_t){TV1_MINUS}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF13;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF14) {
        xQueueSendFromISR(action_queue, &(interface_action_t){START_BUTTON}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF14;
    }

    if (EXTI->PR1 & EXTI_PR1_PIF15) {
        xQueueSendFromISR(action_queue, &(interface_action_t){SELECT_BUTTON}, NULL);
        EXTI->PR1 = EXTI_PR1_PIF15;
    }
}

#define BUTTON_EXTI_MASK (EXTI_IMR1_IM0  | EXTI_IMR1_IM1  | \
                          EXTI_IMR1_IM4  | EXTI_IMR1_IM5  | \
                          EXTI_IMR1_IM6  | EXTI_IMR1_IM7  | \
                          EXTI_IMR1_IM8  | EXTI_IMR1_IM9  | \
                          EXTI_IMR1_IM11 | EXTI_IMR1_IM13 | \
                          EXTI_IMR1_IM14 | EXTI_IMR1_IM15)

void driver_interface_init() {
    INIT_QUEUE(action_queue, interface_action_t, ACTION_QUEUE_LENGTH);

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // PB0, PB1 -> EXTI0, EXTI1
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0 |
                           SYSCFG_EXTICR1_EXTI1);

    SYSCFG->EXTICR[0] |=  (SYSCFG_EXTICR1_EXTI0_PB |
                           SYSCFG_EXTICR1_EXTI1_PB);

    // PA4, PA5, PC6, PC7 -> EXTI4, EXTI5, EXTI6, EXTI7
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI4 |
                           SYSCFG_EXTICR2_EXTI5 |
                           SYSCFG_EXTICR2_EXTI6 |
                           SYSCFG_EXTICR2_EXTI7);

    SYSCFG->EXTICR[1] |=  (SYSCFG_EXTICR2_EXTI4_PA |
                           SYSCFG_EXTICR2_EXTI5_PA |
                           SYSCFG_EXTICR2_EXTI6_PC |
                           SYSCFG_EXTICR2_EXTI7_PC);

    // PC8, PC9, PB11 -> EXTI8, EXTI9, EXTI11
    SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI8 |
                           SYSCFG_EXTICR3_EXTI9 |
                           SYSCFG_EXTICR3_EXTI11);

    SYSCFG->EXTICR[2] |=  (SYSCFG_EXTICR3_EXTI8_PC |
                           SYSCFG_EXTICR3_EXTI9_PC |
                           SYSCFG_EXTICR3_EXTI11_PB);

    // PB13, PB14, PB15 -> EXTI13, EXTI14, EXTI15
    SYSCFG->EXTICR[3] &= ~(SYSCFG_EXTICR4_EXTI13 |
                           SYSCFG_EXTICR4_EXTI14 |
                           SYSCFG_EXTICR4_EXTI15);

    SYSCFG->EXTICR[3] |=  (SYSCFG_EXTICR4_EXTI13_PB |
                           SYSCFG_EXTICR4_EXTI14_PB |
                           SYSCFG_EXTICR4_EXTI15_PB);

    // Clear pending flags before enabling interrupts
    EXTI->PR1 = BUTTON_EXTI_MASK;

    // Unmask interrupts
    EXTI->IMR1 |= BUTTON_EXTI_MASK;

    // Falling edge only
    EXTI->RTSR1 &= ~BUTTON_EXTI_MASK;
    EXTI->FTSR1 |=  BUTTON_EXTI_MASK;

    // NVIC setup
    NVIC_SetPriority(EXTI0_IRQn, 7);
    NVIC_SetPriority(EXTI1_IRQn, 7);
    NVIC_SetPriority(EXTI4_IRQn, 7);
    NVIC_SetPriority(EXTI9_5_IRQn, 7);
    NVIC_SetPriority(EXTI15_10_IRQn, 7);

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
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
            case MARK_DATA: {
                CAN_SEND_mark_data(xTaskGetTickCount(), data_mark_index);
                data_mark_index++;
                break;
            }
            case TOGGLE_REGEN: {
                bool new_regen = !can_data.vcu_settings.is_regen_enabled;
                CAN_SEND_vcu_driver_request(
                    can_data.vcu_settings.vcu_mode,
                   can_data.vcu_settings.lateral_gain,
                    can_data.vcu_settings.longitudinal_gain,
                    can_data.vcu_settings.electronic_brake_bias,
                    new_regen
                );
                break;
            }
            case EBB_MINUS: {
                uint8_t new_bias = CLAMP(can_data.vcu_settings.electronic_brake_bias - 1, 0, 100);
                CAN_SEND_vcu_driver_request(
                    can_data.vcu_settings.vcu_mode,
                   can_data.vcu_settings.lateral_gain,
                    can_data.vcu_settings.longitudinal_gain,
                    new_bias,
                    can_data.vcu_settings.is_regen_enabled
                );
                break;
            }
            case EBB_PLUS: {
                uint8_t new_bias = CLAMP(can_data.vcu_settings.electronic_brake_bias + 1, 0, 100);
                CAN_SEND_vcu_driver_request(
                    can_data.vcu_settings.vcu_mode,
                   can_data.vcu_settings.lateral_gain,
                    can_data.vcu_settings.longitudinal_gain,
                    new_bias,
                    can_data.vcu_settings.is_regen_enabled
                );
                break;
            }
            case TV1_PLUS: {
                uint8_t new_lateral_gain = CLAMP(can_data.vcu_settings.lateral_gain + 1, 0, 100);
                CAN_SEND_vcu_driver_request(
                    can_data.vcu_settings.vcu_mode,
                    new_lateral_gain,
                    can_data.vcu_settings.longitudinal_gain,
                    can_data.vcu_settings.electronic_brake_bias,
                    can_data.vcu_settings.is_regen_enabled
                );
                break;
            }
            case TV1_MINUS: {
                uint8_t new_lateral_gain = CLAMP(can_data.vcu_settings.lateral_gain - 1, 0, 100);
                CAN_SEND_vcu_driver_request(
                    can_data.vcu_settings.vcu_mode,
                    new_lateral_gain,
                    can_data.vcu_settings.longitudinal_gain,
                    can_data.vcu_settings.electronic_brake_bias,
                    can_data.vcu_settings.is_regen_enabled
                );
                break;
            }
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