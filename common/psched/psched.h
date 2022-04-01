#ifndef _PSCHED_H_
#define _PSCHED_H_

// Includes
#if defined(STM32L432xx)
    #include "stm32l4xx.h"
    #include "system_stm32l4xx.h"
#elif defined(STM32L496xx)
    #include "stm32l4xx.h"
    #include "system_stm32l4xx.h"
#elif defined(STM32F4)
    #include "system_stm32f4xx.h"
#else
    #error "Please define a MCU arch"
#endif

#ifndef PSCHED_USE_TIM7
#define P_TIM TIM2
#else
#define P_TIM TIM7
#endif

#define MAX_TASK        15
#define CPU_HIGH        90
#define CPU_MID         75

#define toMicros(time) ((uint32_t) time * 1000)

// Structs, Enums, Types
enum {
    TASK_FG,
    TASK_BG
};

typedef void (*func_ptr_t)(void);

typedef struct {
    uint32_t entry_time_ticks;              // Entry time in OS ticks
    uint32_t exit_time_ticks;               // Exit time in OS ticks
    uint16_t entry_time_cnt;                // Entry time in counter LSB
    uint16_t exit_time_cnt;                 // Exit time in counter LSB
    float    cpu_use;                       // %age of CPU use by this entity
} time_stats_t;

typedef struct {
    time_stats_t fg_stats;                  // Foreground loop stats
    time_stats_t bg_stats;                  // Background loop stats
} cpu_t;

typedef struct
{
    uint8_t      llevel;                    // Logging level
    uint8_t      handler_set;               // Flag for exception handler function
    uint8_t      skips;                     // Number of loop misses. Execution time was skipped. Should always be 0
    uint8_t      run_next;                  // Triggers background to run next iteration
    uint8_t      task_count;                // Number of tasks
    uint8_t      bg_count;                  // Number of background tasks
    uint8_t      running;                   // Marks scheduler as running
    uint32_t     os_ticks;                  // Current OS tick count
    uint32_t     of;                        // MCU operating frequency

    uint16_t     task_time[MAX_TASK];       // Task interval in ms
    func_ptr_t   task_pointer[MAX_TASK];    // Function pointers to tasks
    func_ptr_t   bg_pointer[MAX_TASK];      // Function pointers to background tasks
    func_ptr_t   exception_handler;         // Function pointer to exception handler

    time_stats_t fg_task_stats[MAX_TASK];   // Foreground task stats

    cpu_t    core;                          // Overall loop stats
} sched_t;

extern sched_t sched;

// Prototypes
void taskCreate(func_ptr_t func, uint16_t task_time);
void taskCreateBackground(func_ptr_t func);
void setLogging(uint8_t level, func_ptr_t handler);
void taskDelete(uint8_t type, uint8_t task);
void schedInit(uint32_t freq);
void schedStart(void);
void schedPause(void);

#endif
