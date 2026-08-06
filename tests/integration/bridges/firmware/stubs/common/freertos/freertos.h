#ifndef PER_TEST_FREERTOS_H
#define PER_TEST_FREERTOS_H

#include <stdint.h>

typedef void *QueueHandle_t;

#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define FREERTOS_DEFINE_TASK(...)
#define FREERTOS_START_TASK(...)
#define TASK_PRIORITY_HIGH 0
#define STACK_1024 0
#define STACK_2048 0

extern uint32_t per_test_tick_ms;
static inline uint32_t xTaskGetTickCount(void) {
    return per_test_tick_ms;
}

#endif
