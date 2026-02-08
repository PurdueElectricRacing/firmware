#ifndef PROFILING_H
#define PROFILING_H

#include <stdint.h>

#define PROF_MAX_TASKS 25

typedef struct {
    const char *name;
    uint32_t samples;
} prof_task_t;

void prof_init(void);
void prof_register_task(uint16_t task_id, const char *name);
void prof_sample(uint16_t task_id);
void prof_dump(void);

#endif