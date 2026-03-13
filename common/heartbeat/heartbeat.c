/**
 * @file heartbeat.c
 * @brief Shared heartbeat task implementation for status LEDs.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "heartbeat.h"
#include "common/freertos/freertos.h"

extern uint32_t last_can_rx_time_ms;

static constexpr uint32_t CONN_LED_TIMEOUT_MS = 1000;
static constexpr uint32_t PREFLIGHT_ANIMATION_DURATION_MS = 1500;
// static constexpr uint32_t HEARTBEAT_PERIOD_MS = 100;

void heartbeat_task(status_leds_t *leds) {
    // preflight animation for the first seconds after boot
    if (getTick() <= PREFLIGHT_ANIMATION_DURATION_MS) {
        static uint32_t sweep_index = 0;

        // Creates a sweeping pattern
        switch (sweep_index++ % 3) {
            case 0:
                PHAL_writeGPIO(leds->heartbeat_port, leds->heartbeat_pin, 1);
                PHAL_writeGPIO(leds->connection_port, leds->connection_pin, 0);
                PHAL_writeGPIO(leds->error_port, leds->error_pin, 0);
                break;
            case 1:
                PHAL_writeGPIO(leds->heartbeat_port, leds->heartbeat_pin, 0);
                PHAL_writeGPIO(leds->connection_port, leds->connection_pin, 1);
                PHAL_writeGPIO(leds->error_port, leds->error_pin, 0);
                break;
            case 2:
                PHAL_writeGPIO(leds->heartbeat_port, leds->heartbeat_pin, 0);
                PHAL_writeGPIO(leds->connection_port, leds->connection_pin, 0);
                PHAL_writeGPIO(leds->error_port, leds->error_pin, 1);
                break;
        }

        if (leds->preflight_callback) {
            leds->preflight_callback();
        }

        return;
    }

    PHAL_toggleGPIO(leds->heartbeat_port, leds->heartbeat_pin);

    bool is_can_ok = (getTick() - last_can_rx_time_ms < CONN_LED_TIMEOUT_MS);
    PHAL_writeGPIO(leds->connection_port, leds->connection_pin, is_can_ok);
}

