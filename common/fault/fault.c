/*
 * @file faults.c
 * @author Dawson Moore (moore800@purdue.edu)
 * @brief Vehicle level fault tracking
 * @version 0.1
 * @date 2022-03-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "faults_template.h"

// File scope vars
static fault_core_t core;

// Static function prototypes

void heartTask(void) {
    // TODO: Send heartbeat CAN frame here
}

void faultBg(void) {
    size_t i;
    size_t crit_count;
    size_t warn_count;

    for (i = 0; i < F_COUNT; i++) {
        if (core.faults[i].active) {
            if (f_crit[i]) {
                ++crit_count;
            } else {
                ++warn_count;
            }
        }
    }

    core.master_caution = warn_count ? true : false;
    core.master_error = crit_count ? true : false;
}

void faultUpdate(size_t idx, bool set) {
    size_t time_trigger;
    int8_t delta = set ? 1 : -1;

    if (core.faults[idx].active) {
        delta = -delta;
    }

    if (f_wind[idx]) {
        core.faults[idx].time_curr = CLAMP((int32_t) core.faults[idx].time_curr + delta, 0, (int32_t) core.faults[idx].time_curr + 1);
        core.faults[idx].time_curr += delta;
    } else {
        if (!set) {
            core.faults[idx].active = false;
            core.faults[idx].time_curr = 0;
        } else {
            core.faults[idx].time_curr += delta;
        }
    }

    time_trigger = core.faults[idx].active ? f_ul_time[idx] : f_l_time[idx];

    if (core.faults[idx].time_curr >= time_trigger) {
        core.faults[idx].active = !core.faults[idx].active;
        core.faults[idx].time_curr = 0;
    }
}

void faultForce(size_t idx, bool set) {
    core.faults[idx].active = set;
    core.faults[idx].time_curr = 0;

    if (!set) {
        // TODO: Send a CAN frame. Log the manual unlatch
    }
}

bool faultCheck(size_t idx) {
    return core.faults[idx].active;
}

void __assert(bool truth) {
    if (!truth) {
        __recover();
    }
}

__weak void __recover(void* mem_addr) {
    // TODO: Set stack dump, as well as a few bytes around a specific mem_address
    if (mem_addr != NULL) {

    }

    NVIC_SystemReset();
}
