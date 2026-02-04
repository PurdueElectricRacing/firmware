#include <stdint.h>

#define PROF_MAX_TASKS 25

typedef struct {
    const char *name;
    uint32_t samples;
} prof_task_t;

void profInit();
void profRegisterTask(uint16_t task_id, const char *name);
void profSample(uint16_t task_id);
void profDump(void);