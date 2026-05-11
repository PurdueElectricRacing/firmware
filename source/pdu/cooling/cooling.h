#ifndef COOLING_H
#define COOLING_H

#include <stdint.h>

static constexpr uint32_t COOLING_FSM_PERIOD_MS = 500;
void cooling_fsm_periodic(void);

#endif // COOLING_H
