/**
 * @file faults.h
 * @author Aditya Anand (anand89@purdue.edu)
 * @brief Creating a library of faults to create an easy to debug atmosphere on the car
 * @version 0.1
 * @date 2022-05-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _FAULTS_H_
#define _FAULTS_H_

#include <stdint.h>
#include <stdlib.h>

#include "common/queue/queue.h"

#define MAX_MSG_SIZE 75

extern uint16_t most_recent_latched;

typedef enum {
    FAULT_WARNING = 0,
    FAULT_ERROR   = 1,
    FAULT_FATAL   = 2
} fault_priority_t;

//Contains info about fault state
typedef struct {
    bool latched;
    int f_ID;
} fault_status_t;

//Contains info about the fault as a whole
typedef struct {
    bool tempLatch;
    bool forceActive;
    fault_priority_t priority;
    uint8_t bounces;
    uint16_t time_since_latch;
    int f_max;
    int f_min;
    fault_status_t *status;
    uint32_t start_ticks;
    char *screen_MSG;
} fault_attributes_t;

//Union to package CAN messages
typedef union {
    struct {
        uint64_t idx : 16;
        uint64_t latched : 1;
    } fault_sync;

    uint8_t raw_data[8];
} __attribute__((packed)) fault_can_format_t;

//Function defs
void initFaultLibrary(uint8_t mcu, q_handle_t *txQ, uint32_t ext);
bool setFault(int, int);
static void forceFault(int id, bool state);
static void unForce(int);
static void txFaultSpecific(int);
bool updateFault(uint16_t idx);
void heartBeatTask();
void updateFaults();
void killFaultLibrary();
void handleCallbacks(uint16_t id, bool latched);
bool currMCULatched();
bool warningLatched();
bool errorLatched();
bool fatalLatched();
bool otherMCUsLatched();
bool isLatched();
bool checkFault(int id);

#endif