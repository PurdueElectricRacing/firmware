#include "lcd.h"
#include "strbuf.h"
#include "common/freertos/freertos.h"
#include "common/hearbeat/heartbeat.h"

// Communication queues
ALLOCATE_STRBUF(lcd_tx_buf, 2048);

typedef enum {
    UPDATE_PAGE,
    MENU_UP,
    MENU_DOWN,
    BACK_PAGE,
    FORWARD_PAGE,
    SELECT_BUTTON,
    START_BUTTON
} interface_action_t;
DEFINE_QUEUE(action_queue, interface_action_t, 10);

void action_dispatcher() {
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
            default:
                break;
        }
    }
}

extern status_leds_t status_leds;
void set_external_leds() {
    // dont update the external LEDS until we're out of preflight
    if (status_leds.state == HEARTBEAT_STATE_PREFLIGHT) {
        return;
    }

    bool precharge_incomplete = is_latched(FAULT_ID_PRECHARGE_INCOMPLETE);
    PHAL_writeGPIO(PRCHG_LED_PORT, PRCHG_LED_PIN, !precharge_incomplete);

}

void update_telemetry() {
    // todo
}

void driver_interface_periodic() {
    action_dispatcher(); // Process Pending Actions

    update_telemetry();

    LCD_tx_update(); // dump the command

    set_external_leds();
}