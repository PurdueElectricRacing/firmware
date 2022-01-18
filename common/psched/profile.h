#ifndef _PROFILE_H_
#define _PROFILE_H_

// Uncomment depending on MCU type
#if !defined(L4) || !defined(F4)
    #define L4
    // #define F4
#endif

// Includes
#if defined(L4)
    #include "stm32l4xx.h"
    #include "system_stm32l4xx.h"
#elif defined(F4)
    #include "system_stm32f4xx.h"
#endif

#include "stdbool.h"

// Structs, Enums, Types
typedef struct {
    uint16_t task_time;                 // Time spent in a task
    uint16_t bg_time;                   // Time spent in bg loop
    uint32_t task_entry_time;           // Tick time of task entry
    uint32_t bg_entry_time;             // Tick time of background entry
    float    cpu_use;                   // % use of task time
} cpu_t;

typedef struct {
    uint8_t skips;                      // Number of loop misses. Execution time was skipped. Should always be 0
    cpu_t   core;
} profile_t;

#endif