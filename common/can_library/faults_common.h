#ifndef FAULTS_COMMON_H
#define FAULTS_COMMON_H

/**
 * @file faults_common.h
 * @brief Creating a library of faults to create an easy to debug atmosphere on the car
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Aditya Anand (anand89@purdue.edu)
 */

#include <stdint.h>
#include <stdbool.h>
#include "common/can_library/generated/can_types.h"

typedef enum {
    FAULT_PRIO_WARNING,
    FAULT_PRIO_ERROR,
    FAULT_PRIO_FATAL,
    NUM_FAULT_PRIOS
} fault_priority_t;

typedef struct {
    uint16_t latch_period;
    uint16_t max_value;
    uint16_t min_value;
    uint16_t latch_ms;
    uint16_t unlatch_ms;
    fault_priority_t priority;
} fault_attributes_t;

typedef struct {
    uint32_t start_ticks;
    bool temp_state;
    bool is_latched;
} fault_status_t;

// API
bool set_fault(fault_index_t idx, uint16_t value);
bool is_latched(fault_index_t idx);
void fault_library_periodic();

bool warning_latched();
bool error_latched();
bool fatal_latched();
bool curr_mcu_latched();
bool other_mcus_latched();
bool is_any_latched();

#ifdef HAS_FAULT_STRINGS
const char* get_fault_string(fault_index_t idx);
#endif

#endif // FAULTS_COMMON_H
