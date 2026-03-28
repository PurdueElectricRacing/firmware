/**
 * @file freertos.c
 * @brief Wrapper macros for FreeRTOS constructs (tasks, queues, semaphores) to simplify static memory allocation and initialization.
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 */

#include "freertos.h"

void periodic_task_runner(void *arg) {
    periodic_task_params_t *wrapper = (periodic_task_params_t *)arg;

    while (true) {
        wrapper->taskFunction();
        osDelay(wrapper->period_ms);
    }

    osThreadTerminate(NULL);
}