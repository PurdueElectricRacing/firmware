
#include "freertos.h"

void rtosWrapper(void *thread) {
    ThreadWrapper_t *wrapper = (ThreadWrapper_t *)thread;

    while (true) {
        wrapper->taskFunction();
        osDelay(wrapper->period_ms);
    }

    osThreadTerminate(NULL);
}
