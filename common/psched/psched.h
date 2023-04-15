#ifndef _PSCHED_H_
#define _PSCHED_H_

// Includes
#if defined(STM32L432xx)
#include "stm32l4xx.h"
#include "system_stm32l4xx.h"
#elif defined(STM32L471xx)
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

#define toMicros(time) ((uint32_t) time * 1000)

#define ARR_SET         1000
#define MAX_TASKS       32
#define E_NO_FREE_TASK  1
#define E_FG_MISS       2
#define E_BG_MISS       3
#define E_IND_FG_MISS   4

// Structs, Enums, Types
enum {
    TASK,
    TASK_BG
};

typedef void (*func_ptr_t)(void);

typedef struct {
    uint32_t tick_entry;                    // OS tick count on entry
    uint32_t tick_exit;                     // OS tick count on exit
    uint32_t cnt_entry;                     // TIM7->CNT on entry
    uint32_t cnt_exit;                      // TIM7->CNT on exit

    float    cpu_use;                       // Instantaneous CPU use
    float    max_cpu_use;                   // Maximum CPU use (excluding timing misses)
    uint8_t  has_missed;                    // Marks if a task has missed at some point
} cpu_time_t;

typedef struct
{
    uint8_t    skips;                       // Number of loop misses. Execution time was skipped. Should always be 0
    uint8_t    run_next;                    // Triggers background to run next iteration
    uint8_t    fg_count;                    // Number of foreground tasks
    uint8_t    bg_count;                    // Number of background tasks
    uint8_t    running;                     // Marks scheduler as running
    uint8_t    preflight_required;          // Preflight in use
    uint8_t    preflight_complete;          // Preflight complete registration
    uint8_t    anim_complete;               // Preflight setup/inspection complete
    uint8_t    error;                       // Core error flags
    uint16_t   anim_min_time;               // Minimum runtime of preflight animation
    uint32_t   os_ticks;                    // Current OS tick count
    uint32_t   of;                          // MCU operating frequency

    uint16_t   task_time[MAX_TASKS];        // Timings of registered tasks
    uint16_t   anim_time;                   // Timing of animation
    func_ptr_t task_pointer[MAX_TASKS];     // Function pointers to tasks
    func_ptr_t bg_pointer[MAX_TASKS];       // Function pointers to background tasks

    func_ptr_t anim;                        // Animation function
    func_ptr_t preflight;                   // Preflight function

    cpu_time_t fg_time;                     // Foreground time use
    cpu_time_t bg_time;                     // Background time use
    cpu_time_t ind_fg_time[MAX_TASKS];      // Individual foreground task time use
} sched_t;

extern sched_t sched;

// Prototypes
int  taskCreate(func_ptr_t func, uint16_t task_time);
int  taskCreateBackground(func_ptr_t func);
void taskDelete(uint8_t type, uint8_t task);
void configureAnim(func_ptr_t anim, func_ptr_t preflight, uint16_t anim_time, uint16_t anim_min_time);
void registerPreflightComplete(uint8_t status);
void schedInit(uint32_t freq);
void schedStart();
void schedPause();

#endif