/**
 * @file faults_common.c
 * @brief Creating a library of faults to create an easy to debug atmosphere on the car
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Aditya Anand (anand89@purdue.edu)
 */

#include "common/can_library/faults_common.h"
#include "common/can_library/generated/fault_data.h"
#include "common/can_library/generated/can_router.h"
#include "common/psched/psched.h"

static uint16_t fault_counters[NUM_FAULT_PRIOS];

inline bool is_latched(fault_index_t idx) {
    if (idx >= TOTAL_NUM_FAULTS) {
        return false;
    }

    return faults[idx].state == FAULT_STATE_LATCHED;
}

void update_fault(fault_index_t idx, uint16_t value) {
    if (idx >= TOTAL_NUM_FAULTS) {
        return;
    }

    fault_t *fault = &faults[idx];
    uint32_t now = OS_TICKS;

    bool is_out_of_bounds = (value > fault->max_value) || (value < fault->min_value);

    switch(fault->state) {
        case FAULT_STATE_OK: {
            if (is_out_of_bounds) {
                fault->state = FAULT_STATE_PENDING;
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
            }
            break;

        }
        case FAULT_STATE_LATCHED: {
            if (!is_out_of_bounds) {
                fault->state = FAULT_STATE_RECOVERING;
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
        if (faults[i].is_latched) {
            fault_counters[fault_attributes[i].priority]++;
        }
    }
    
    // Broadcast our local state to the rest of the car
    tx_fault_sync();
}

bool warning_latched() {
    return fault_counters[FAULT_PRIO_WARNING] > 0;
}

bool error_latched() {
    return fault_counters[FAULT_PRIO_ERROR] > 0;
}

bool fatal_latched() {
    return fault_counters[FAULT_PRIO_FATAL] > 0;
}

#ifdef HAS_FAULT_STRINGS
const char* get_fault_string(fault_index_t idx) {
    if (idx >= TOTAL_NUM_FAULTS) return nullptr;
    return fault_strings[idx];
}
#endif

bool curr_mcu_latched() {
#ifdef MY_FAULT_START
    for (int i = MY_FAULT_START; i <= MY_FAULT_END; i++) {
        if (faults[i].is_latched) return true;
    }
#endif
    return false;
}

bool other_mcus_latched() {
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
#ifdef MY_FAULT_START
        if (i >= MY_FAULT_START && i <= MY_FAULT_END) continue;
#endif
        if (faults[i].is_latched) return true;
    }
    return false;
}

bool is_any_latched() {
    return warning_latched() || error_latched() || fatal_latched();
}

