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
static void checkCallback(size_t idx);

void faultInit(err_callback callback) {
    core.__callback_set = (callback == NULL) ? false : true;
    core.callback = callback;
}

void faultsBg(void) {

}

void faultUpdate(size_t idx, bool set) {
    int8_t delta = set ? 1 : -1;

    if (core.faults[idx].active) {
        delta = -delta;
    }

    if (f_wind[idx]) {
        core.faults[idx].time_curr += delta;
    } else {
        if (!set) {
            core.faults[idx].active = false;
            core.faults[idx].time_curr = 0;
        } else {
            core.faults[idx].time_curr += delta;
        }
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

static void checkCallback(size_t idx) {

}