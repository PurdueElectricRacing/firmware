/**
 * @file faults_common.c
 * @brief Event-based FIDR system
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "common/can_library/faults_common.h"

#include "common/can_library/generated/can_router.h"
#include "common/can_library/generated/fault_data.h"

#ifdef FAULT_LIB_ENABLED

static uint16_t fault_counters[NUM_FAULT_PRIOS];

#ifdef MY_FAULT_START
static_assert(MY_FAULT_START <= MY_FAULT_END);
static_assert(MY_FAULT_END < TOTAL_NUM_FAULTS);
#else
#error "Fault library misconfigured"
#endif

bool is_latched(fault_index_t fault_index) {
    if (fault_index >= TOTAL_NUM_FAULTS) {
        return false;
    }

    return faults[fault_index].state == FAULT_STATE_LATCHED;
}

void update_fault(fault_index_t fault_index, uint16_t value) {
    if ((fault_index < MY_FAULT_START) || (fault_index > MY_FAULT_END)) {
        return;
    }

    fault_t *fault        = &faults[fault_index];
    uint32_t now          = OS_TICKS;
    bool is_out_of_bounds = (value > fault->max_value) || (value < fault->min_value);

    // Implementation of the FSM diagram in common/can_library/README
    // I know this FSM is not "mathematically pure", but it must be implemented in this way
    // to satisfy timing constraints/defensive programming
    switch (fault->state) {
        case FAULT_STATE_OK: {
            if (is_out_of_bounds) {
                fault->state         = FAULT_STATE_PENDING;
                fault->start_time_ms = now;
            }
            break;
        }
        case FAULT_STATE_PENDING: {
            if (!is_out_of_bounds) {
                fault->state = FAULT_STATE_OK;
                break;
            }

            uint32_t elapsed = now - fault->start_time_ms;
            // do not update the start_time
            if (elapsed > fault->latch_time_ms) {
                fault->state = FAULT_STATE_LATCHED;
                tx_fault_event(fault_index, value);
            }
            break;
        }
        case FAULT_STATE_LATCHED: {
            if (!is_out_of_bounds) {
                fault->state         = FAULT_STATE_RECOVERING;
                fault->start_time_ms = now;
            }

            break;
        }
        case FAULT_STATE_RECOVERING: {
            if (is_out_of_bounds) {
                fault->state = FAULT_STATE_LATCHED;
                break;
            }

            uint32_t elapsed = now - fault->start_time_ms;
            // do not update the start_time
            if (elapsed > fault->unlatch_time_ms) {
                fault->state = FAULT_STATE_OK;
            }
            break;
        }
    }
}

void fault_library_periodic() {
    // Reset priority counters
    for (int i = 0; i < NUM_FAULT_PRIOS; i++) {
        fault_counters[i] = 0;
    }

    // Count latched faults
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
        if (is_latched(i)) {
            fault_counters[faults[i].priority]++;
        }
    }

    // Broadcast our local state to the rest of the car
    tx_fault_sync();
}

bool is_warning_latched() {
    return fault_counters[FAULT_PRIO_WARNING] > 0;
}

bool is_error_latched() {
    return fault_counters[FAULT_PRIO_ERROR] > 0;
}

bool is_fatal_latched() {
    return fault_counters[FAULT_PRIO_FATAL] > 0;
}

bool is_any_latched() {
    return is_warning_latched() || is_error_latched() || is_fatal_latched();
}

#ifdef HAS_FAULT_STRINGS
const char *get_fault_string(fault_index_t idx) {
    if (idx >= TOTAL_NUM_FAULTS)
        return nullptr;
    return fault_strings[idx];
}
#endif

bool is_curr_mcu_latched() {
#ifdef MY_FAULT_START
    for (int i = MY_FAULT_START; i <= MY_FAULT_END; i++) {
        if (is_latched(i))
            return true;
    }
#endif
    return false;
}

bool is_other_mcus_latched() {
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
#ifdef MY_FAULT_START
        if (i >= MY_FAULT_START && i <= MY_FAULT_END)
            continue;
#endif
        if (is_latched(i))
            return true;
    }
    return false;
}

#endif // FAULT_LIB_ENABLED
