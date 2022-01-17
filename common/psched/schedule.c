#include "common/psched/schedule.h"

static void cswitch(task_info_t* from, task_info_t* to);

void schedule() {
    disable_irq();
}

static void cswitch(task_info_t* from, task_info_t* to) {
    enable_irq();
}