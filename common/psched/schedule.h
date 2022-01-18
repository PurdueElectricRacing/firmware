#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

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
enum {
    TASK,
    TASK_BG
};

typedef void (*func_ptr_t)(void);

typedef struct
{
    uint8_t     task_count;             // Number of tasks
    uint8_t     bg_count;               // Number of background tasks
    uint8_t     running;                // Marks scheduler as running
    uint8_t     fg_running;             // Marks foreground task as running
    uint32_t    of;                     // MCU operating frequency

    uint32_t     task_time[15];         // Timings of registered tasks
    task_info_t* task_pointer[15];      // Function pointers to foreground tasks
    task_info_t* bg_pointer[15];        // Function pointers to background tasks
} sched_t;

extern sched_t sched;

// Prototypes
void scheduleFg(void);
void scheduleBg(void);

#endif