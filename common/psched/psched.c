#include "psched.h"
#include <stddef.h>

static void schedLoop(void);
static void schedBg(void);
static void memsetu(uint8_t* ptr, uint8_t val, size_t size);
static void calcStats(void);

sched_t sched;

// @funcname: taskCreate()
//
// @brief: Adds task to scheduler with set rate
//
// @param: func: Pointer to function to run 
// @param: task_time: Rate of task in ms
void taskCreate(func_ptr_t func, uint16_t task_time)
{
    sched.task_time[sched.task_count] = task_time;
    sched.task_pointer[sched.task_count++] = func;
}

// @funcname: taskCreateBackground()
//
// @brief: Adds background task to scheduler
//
// @param: func: Pointer to function to run
void taskCreateBackground(func_ptr_t func)
{
    sched.bg_pointer[sched.bg_count++] = func;
}

// @funcname: setLoggingLevel()
//
// @brief: Sets level of CAN logging for task timing
//
// @param: level: 0 for no logging, 1 for critical errors only, 2 for everything
void setLogging(uint8_t level, func_ptr_t handler)
{
    sched.llevel = level;
    sched.exception_handler = handler;
    sched.handler_set = 1;
}

// @funcname: taskDelete()
//
// @brief: Removes task from scheduler
//
// @param: type: 0 for normal task, 1 for background task
// @param: task: Task idx
void taskDelete(uint8_t type, uint8_t task)
{
    // Locals
    uint8_t i;
    func_ptr_t* fp;

     if (type == TASK_FG)
     {
         fp = sched.task_pointer;
        
         for (i = task; i < sched.task_count; i++)
         {
             sched.task_time[i] = sched.task_time[i + 1];
         }
     }
     else
     {
         fp = sched.bg_pointer;
     }

     for (i = task; i < sched.task_count; i++)
     {
         fp[i] = fp[i + 1];
     }

     --sched.task_count;
}

// @funcname: schedInit()
//
// @brief: Initializes the scheduler
//
// @param: freq: Frequency of MCU in Hz
void schedInit(uint32_t freq)
{
    /*
        Note: This system uses timer 2 and the watchdog peripheral
        If you want/need to use this timer, it is on you to edit the configuration
        to use a different timer.
        DO NOT ATTEMPT TO CONFIGURE THIS TIMER IN CUBE!
        The configuration for this timer is done manually, right here.
        The watchdog will trigger a reset if all loops take longer than 3 ms to return

        Also, functions need to return to work properly. The scheduler works on a timer interrupt.
        If the functions called in the timer interrupt, you're going to have a stack overflow.

        Frequencies are given in OS ticks.
    */

    // Configure timer 2
    // Targeting an interrupt every 1 ms
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    TIM2->PSC = (freq / 1000000) - 1;
    TIM2->ARR = 1000;
    TIM2->CR1 &= ~(TIM_CR1_DIR);
    TIM2->DIER |= TIM_DIER_UIE;

    // Default all values
    memsetu((uint8_t*) &sched, 0, sizeof(sched));
    sched.of = freq;
}

// @funcname: schedLoop()
//
// @brief: Main loop that'll run each task
static void schedLoop(void)
{
    // Locals
    uint8_t i;

    while (sched.running == 1)
    {
        // Prep iteration
        sched.run_next = 0;
        IWDG->KR = 0xAAAA;

        // Store fg entry
        sched.core.fg_stats.entry_time_cnt = TIM2->CNT;
        sched.core.fg_stats.entry_time_ticks = sched.os_ticks;

        // Execute tasks (fg entry point)
        for (i = 0; i < sched.task_count; i++)
        {
            if (sched.os_ticks % sched.task_time[i] == 0)
            {
                sched.fg_task_stats[i].entry_time_cnt = TIM2->CNT;
                sched.fg_task_stats[i].entry_time_ticks = sched.os_ticks;
                (*sched.task_pointer[i])();
                sched.fg_task_stats[i].exit_time_cnt = TIM2->CNT;
                sched.fg_task_stats[i].exit_time_ticks = sched.os_ticks;
            }
        }

        // Store fg exit
        sched.core.fg_stats.exit_time_cnt = TIM2->CNT;
        sched.core.fg_stats.exit_time_ticks = sched.os_ticks;

        // Check if we missed timing requirements
        if (sched.run_next == 1)
        {
            ++sched.skips;
        }

        // Store bg entry
        sched.core.bg_stats.entry_time_cnt = TIM2->CNT;
        sched.core.bg_stats.entry_time_ticks = sched.os_ticks;

        // Bg entry point
        schedBg();

        // Store bg exit
        sched.core.bg_stats.entry_time_cnt = TIM2->CNT;
        sched.core.bg_stats.entry_time_ticks = sched.os_ticks;

        // Calculate task runtimes
        calcStats();
    }
}

// @funcname: schedBg()
//
// @brief: Background loop running when nothing else is
static void schedBg(void)
{
    // Locals
    uint8_t i;

    while (1)
    {
        // Check if we should break
        if (sched.run_next == 1)
        {
            return;
        }

        // Execute background tasks
        for (i = 0; i < sched.bg_count; i++)
        {
            (*sched.bg_pointer[i])();
        }
    }
}

// @funcname: waitMicros()
//
// @brief: Waits for set time in microseconds,
//         but never for more than 100μs at a time
//
// @param: time: time to wait in μs
void waitMicros(uint8_t time)
{
    if (time > 100) time = 100;

    uint16_t entry_time = TIM2->CNT; // ARR is 1k, so no cast issues
    int16_t  exit_time = (int16_t) entry_time - time;

    if (exit_time < 0)
    {
        exit_time += 1000;
        while (TIM2->CNT < entry_time);
    }

    while (TIM2->CNT > exit_time);
}

// @funcname: schedStart()
//
// @brief: Starts tasks. Will never return
void schedStart(void)
{
    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1 << TIM2_IRQn;
    IWDG->KR  =  0xCCCC;     
    IWDG->KR  =  0x5555;
    IWDG->PR  |= 2;
    IWDG->RLR =  20;
    sched.running = 1;

    while ((IWDG->SR & 0b111) != 0);

    IWDG->KR = 0xAAAA;

    schedLoop();
}

// @funcname: schedPause()
//
// @brief: Stops scheduling and allows schedStart() to return
//         Does not need re-initialization after calling
void schedPause(void)
{
    TIM2->CR1 &= ~TIM_CR1_CEN;
    NVIC->ISER[0] &= ~(1 << TIM2_IRQn);
    sched.running = 0;
    sched.run_next = 1;
}

// @funcname: memsetu
//
// @brief: Simple memset routine
//
// @param: ptr: Pointer to location to set
// @param: val: Value to set each memory address to
// @param: size: Length of data to set
static void memsetu(uint8_t* ptr, uint8_t val, size_t size)
{
    // Locals
    uint8_t i;

    for (i = 0; i < size; i++)
    {
        ptr[i] = val;
    }
}

// @funcname: statsHelper
//
// @brief: Runs actual calculation
static void statsHelper(time_stats_t* stats)
{
    int16_t ttime;

    if (stats->entry_time_ticks != stats->exit_time_ticks)
    {
        stats->cpu_use = 100;
    }
    else
    {
        ttime = stats->exit_time_cnt - stats->entry_time_cnt;

        if (ttime < 0)
        {
            ttime += 1000;
        }

        stats->cpu_use = ((float) ttime) / 10;
    }

    if (!sched.handler_set)
    {
        return;
    }

    if (stats->cpu_use > CPU_HIGH && sched.llevel)
    {
        sched.exception_handler();
    }
    else if (stats->cpu_use > CPU_MID && sched.llevel == 2)
    {
        sched.exception_handler();
    }
}

// @funcname: calcStats
//
// @brief: Calculates percent runtime of each task
static void calcStats(void)
{
    uint8_t i;

    statsHelper(&sched.core.fg_stats);
    statsHelper(&sched.core.bg_stats);

    for (i = 0; i < MAX_TASK; i++)
    {
        statsHelper(&sched.fg_task_stats[i]);
    }
}

// @funcname: TIM7_IRQHandler()
//
// @brief: Timer 7 IRQ. Increments OS ticks and unblocks loop
void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~TIM_SR_UIF;

    if (++sched.os_ticks == 30000)
    {
        sched.os_ticks = 0;
    }

    sched.run_next = 1;
}
