/**
 * @file cooling.h
 * @brief thermal control state machine implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef COOLING_FSM_H
#define COOLING_FSM_H

#include <stdint.h>

static constexpr uint32_t COOLING_FSM_PERIOD_MS = 500;
void cooling_fsm_periodic(void);

#endif // COOLING_FSM_H
