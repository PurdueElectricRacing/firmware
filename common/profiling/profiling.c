#include "profiling.h"
#include <stdint.h>

static prof_task_t *prof_tasks = {0};
static uint32_t prof_total_samples = 0;
prof_task_t prof_tasks_array[PROF_MAX_TASKS] = {0};

void prof_init(void)
{
    for (int32_t i = 0; i < PROF_MAX_TASKS; i++) {
        prof_tasks[i].name = 0;
        prof_tasks[i].samples = 0;
    }
    prof_total_samples = 0;
}

void prof_register_task(uint16_t task_id, const char *name)
{
    prof_tasks[task_id].name = name;
}

void prof_sample(uint16_t task_id)
{
    prof_tasks[task_id].samples++;
    prof_total_samples++;
}

void prof_dump(void)
{
    for (int32_t i = 0; i < PROF_MAX_TASKS; i++) {
        if (prof_tasks[i].name) {
            float pct = (100.0f * prof_tasks[i].samples) / prof_total_samples;
            // do something with this
        }
    }
}
