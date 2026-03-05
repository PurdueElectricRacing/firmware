/**
 * @file freertos.c
 * @brief Wrapper macros for FreeRTOS constructs (tasks, queues, semaphores) to simplify static memory allocation and initialization.
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 */

#include "freertos.h"

void rtosWrapper(void *thread) {
    ThreadWrapper_t *wrapper = (ThreadWrapper_t *)thread;

    while (true) {
        wrapper->taskFunction();
        osDelay(wrapper->period_ms);
    }

    osThreadTerminate(NULL);
}
