/**
 * @file freertos.c
 * @brief Native FreeRTOS wrapper implementation.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Millan Kumar (kumar798@purdue.edu)
 */

#include "freertos.h"

void freertos_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void periodic_task_runner(void *arg) {
    periodic_task_params_t *wrapper = (periodic_task_params_t *)arg;

    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(wrapper->period_ms);

    while (true) {
        wrapper->taskFunction();
        vTaskDelayUntil(&lastWakeTime, period);
    }

    // Unreachable!
    vTaskDelete(NULL);
}