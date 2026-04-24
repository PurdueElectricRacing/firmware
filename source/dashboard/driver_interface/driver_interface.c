#include "can_library/faults_common.h"
#include "can_library/generated/DASHBOARD.h"
#include "common/freertos/freertos.h"
#include "common/heartbeat/heartbeat.h"
#include "common/phal/usart.h"

#include "lcd.h"
#include "main.h"
#include "strbuf.h"

typedef enum {
    UPDATE_PAGE,
    MENU_UP,
    MENU_DOWN,
    BACK_PAGE,
    FORWARD_PAGE,
    SELECT_BUTTON,
    START_BUTTON,
} interface_action_t;
DEFINE_QUEUE(action_queue, interface_action_t, 10);

void EXTI9_5_IRQHandler() {
    // EXTI9 (LEFT Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF9) {
        xQueueSendFromISR(action_queue, &(interface_action_t){FORWARD_PAGE}, NULL);
        EXTI->PR1 |= EXTI_PR1_PIF9;
    }

    // EXTI8 (RIGHT Button) triggered the interrupt
    if (EXTI->PR1 & EXTI_PR1_PIF8) {
        xQueueSendFromISR(action_queue, &(interface_action_t){BACK_PAGE}, NULL);
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
                CAN_SEND_start_button(true);
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

// Communication queues
ALLOCATE_STRBUF(lcd_tx_buf, 2048);

/**
 * @brief Called periodically to send commands to the Nextion LCD display via USART
 */
void LCD_tx_update(void) {
    if (PHAL_usartTxBusy(&lcd)) {
        return;
    }

    if (lcd_tx_buf.length == 0) {
        return;
    }

    PHAL_usartTxDma(&lcd, (uint8_t *)lcd_tx_buf.data, lcd_tx_buf.length);
    strbuf_clear(&lcd_tx_buf);
}

void driver_interface_periodic(void) {
    action_dispatcher(); // Process Pending Driver Actions

    updateTelemetryPages();

    LCD_tx_update(); // dump the command

    set_external_leds();
}