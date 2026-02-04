#include "profiling.h"
#include <stdint.h>

static prof_task_t *prof_tasks;
static uint32_t prof_total_samples;
prof_task_t prof_tasks_array[PROF_MAX_TASKS];

void profInit() {
    for (int i = 0; i < PROF_MAX_TASKS; i++) {
        prof_tasks[i].name = 0;
        prof_tasks[i].samples = 0;
    }
    prof_total_samples = 0;
}

void profRegisterTask(uint16_t task_id, const char *name) {
    prof_tasks[task_id].name = name;
}

void profSample(uint16_t task_id) {
    prof_tasks[task_id].samples++;
    prof_total_samples++;
}

void profDump() {
    for (int i = 0; i < PROF_MAX_TASKS; i++) {
        if (prof_tasks[i].name) {
            float pct = (100.0f * prof_tasks[i].samples) / prof_total_samples;
            // do something with this
        }
    }
}
