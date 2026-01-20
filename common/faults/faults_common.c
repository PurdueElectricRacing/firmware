/**
 * @file faults_common.c
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Aditya Anand (anand89@purdue.edu)
 * 
 * Creating a library of faults to create an easy to debug atmosphere on the car
 */

#include "common/faults/faults_common.h"
#include "common/can_library/generated/fault_data.h"
#include "common/can_library/generated/can_router.h"
#include "common/psched/psched.h"

static uint16_t fault_priorities[NUM_FAULT_PRIOS];

bool is_latched(fault_index_t idx) {
    if (idx >= TOTAL_NUM_FAULTS) return false;
    return faults[idx].is_latched;
}

bool set_fault(fault_index_t idx, uint16_t value) {
    if (idx >= TOTAL_NUM_FAULTS) return false;

    const fault_attributes_t *attr = &fault_attributes[idx];
    fault_status_t *status = &faults[idx];

    bool current_out_of_bounds = (value >= attr->max_value || value < attr->min_value);

    uint32_t now = sched.os_ticks;

    if (current_out_of_bounds) {
        if (!status->temp_state) {
            // Entered out-of-bounds state
            status->temp_state = true;
            status->start_ticks = now;
            
            // If latch time is 0, latch immediately
            if (attr->latch_ms == 0 && !status->is_latched) {
                status->is_latched = true;
                tx_fault_event(idx, value);
            }
        } else {
            // Sustained out-of-bounds state
            if (!status->is_latched && (now - status->start_ticks) >= attr->latch_ms) {
                status->is_latched = true;
                tx_fault_event(idx, value);
            }
        }
    } else {
        // Value is within limits
        if (status->temp_state) {
            // Entered within-limits state
            status->temp_state = false;
            status->start_ticks = now;
            
            // If unlatch time is 0, unlatch immediately
            if (attr->unlatch_ms == 0 && status->is_latched) {
                status->is_latched = false;
                tx_fault_event(idx, value);
            }
        } else {
            // Sustained within-limits state
            if (status->is_latched && (now - status->start_ticks) >= attr->unlatch_ms) {
                status->is_latched = false;
                tx_fault_event(idx, value);
            }
        }
    }

    return status->is_latched;
}

void fault_library_periodic() {
    // Reset priority counters
    for (int i = 0; i < NUM_FAULT_PRIOS; i++) {
        fault_priorities[i] = 0;
    }

    // Count latched faults
    for (int i = 0; i < TOTAL_NUM_FAULTS; i++) {
        if (faults[i].is_latched) {
            fault_priorities[fault_attributes[i].priority]++;
        }
    }
    
    // Broadcast our local state to the rest of the car
    tx_fault_sync();
}

bool warning_latched() {
    return fault_priorities[FAULT_PRIO_WARNING] > 0;
}

bool error_latched() {
    return fault_priorities[FAULT_PRIO_ERROR] > 0;
}

bool fatal_latched() {
    return fault_priorities[FAULT_PRIO_FATAL] > 0;
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

