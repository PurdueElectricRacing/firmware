#ifndef CHARGING_FSM_H
#define CHARGING_FSM_H

/**
 * @file charging_fsm.h
 * @brief Charger control state machine implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

#include "can_library/generated/A_BOX.h"

static constexpr uint32_t CHARGING_FSM_PERIOD_MS = 1000;

// static assert that the FSM flushes the CAN messages it owns at least as fast as their defined periods
static_assert(ELCON_COMMAND_PERIOD_MS == CHARGING_FSM_PERIOD_MS);

void charging_fsm_periodic(void);

#endif // CHARGING_FSM_H