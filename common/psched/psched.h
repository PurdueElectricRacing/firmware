#ifndef _PSCHED_H_
#define _PSCHED_H_

// Uncomment depending on MCU type
#define L4
// #define F4

// Includes
#if defined(L4)
    #include "stm32l4xx.h"
    #include "system_stm32l4xx.h"
#elif defined(F4)
    #include "system_stm32f4xx.h"
#endif

// Structs, Enums, Types
enum {
    TASK,
    TASK_BG
};

typedef void (*func_ptr_t)(void);

typedef struct {
    uint16_t task_time;                 // Time spent in a task
    uint16_t bg_time;                   // Time spent in bg loop
    uint32_t task_entry_time;           // Tick time of task entry
    uint32_t bg_entry_time;             // Tick time of background entry
    float    cpu_use;                   // % use of task time
} cpu_t;

typedef struct
{
    uint8_t    skips;                   // Number of loop misses. Execution time was skipped. Should always be 0
    uint8_t    run_next;                // Triggers background to run next iteration
    uint8_t    task_count;              // Number of tasks
    uint8_t    bg_count;                // Number of background tasks
    uint8_t    running;                 // Marks scheduler as running
    uint32_t   os_ticks;                // Current OS tick count
    uint32_t   of;                      // MCU operating frequency

    uint16_t   task_time[15];           // Timings of registered tasks
    func_ptr_t task_pointer[15];        // Function pointers to tasks
    func_ptr_t bg_pointer[15];          // Function pointers to background tasks

    cpu_t    core;
} sched_t;

extern sched_t sched;

// Prototypes
void taskCreate(func_ptr_t func, uint16_t task_time);
void taskDelete(uint8_t type, uint8_t task);
void schedInit(uint32_t freq);
void schedStart();
void schedPause();

#endif
