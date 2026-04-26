#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <stdint.h>

static constexpr uint32_t CONTROL_LOOP_PERIOD_MS = 10;
void control_loop(void);

#endif // CONTROL_LOOP_H