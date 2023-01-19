#include "psched.h"
#include <stddef.h>

static void checkPreflightEnd(void);
static void schedLoop(void);
static void schedBg(void);
static void updateTime(cpu_time_t* time);
static void calcTime(cpu_time_t* time, uint8_t count, int type);
static void memsetu(uint8_t* ptr, uint8_t val, size_t size);

sched_t sched;

// @funcname: taskCreate()
//
// @brief: Adds task to scheduler with set rate
//
// @param: func: Pointer to function to run 
// @param: task_time: Rate of task in ms
int taskCreate(func_ptr_t func, uint16_t task_time)
{
    if (sched.fg_count != MAX_TASKS)
    {
        sched.task_time[sched.fg_count] = task_time;
        sched.task_pointer[sched.fg_count++] = func;

        return 0;
    }

    return -E_NO_FREE_TASK;
}

// @funcname: taskCreateBackground()
//
// @brief: Adds background task to scheduler
//
// @param: func: Pointer to function to run
int taskCreateBackground(func_ptr_t func)
{
    if (sched.bg_count != MAX_TASKS)
    {
        sched.bg_pointer[sched.bg_count++] = func;

        return 0;
    }

    return -E_NO_FREE_TASK;
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

     if (type == TASK)
     {
         fp = sched.task_pointer;
        
         for (i = task; i < sched.fg_count; i++)
         {
             sched.task_time[i] = sched.task_time[i + 1];
         }
     }
     else
     {
         fp = sched.bg_pointer;
     }

     for (i = task; i < sched.fg_count; i++)
     {
         fp[i] = fp[i + 1];
     }

     --sched.fg_count;
}

// @funcname: configureAnim()
//
// @brief: Sets preflight animation functions,
//         and durations
//
// @param: anim: Address of animation function
//               to be scheduled every anim_time ms
// @param: preflight: Address of preflight checks
// @param: anim_time: Animation scheduling time in ms
// @param: anim_min_time: Minimum animation duration in ms
void configureAnim(func_ptr_t anim, func_ptr_t preflight, uint16_t anim_time, uint16_t anim_min_time)
{
    sched.preflight_required = 1;
    sched.anim_time = anim_time;
    sched.anim_min_time = anim_min_time;

    sched.anim = anim;
    sched.preflight = preflight;
}

// @funcname: registerPreflightComplete()
//
// @brief: Signals to PSched that all preflight
//         checks are complete
void registerPreflightComplete(uint8_t status)
{
    sched.preflight_complete = status;
}

// @funcname: schedInit()
//
// @brief: Initializes the scheduler
//
// @param: freq: Frequency of MCU in Hz
void schedInit(uint32_t freq)
{
    /*
        Note: This system uses timer 7 and the watchdog peripheral
        If you want/need to use this timer, it is on you to edit the configuration
        to use a different timer.
        DO NOT ATTEMPT TO CONFIGURE THIS TIMER IN CUBE (or on your own)!
        The configuration for this timer is done manually, right here.
        The watchdog will trigger a reset if all loops take longer than 10 ms to return
        Also, functions need to return to work properly. The scheduler works on a timer interrupt.
        If the functions called in the timer interrupt, you're going to have a stack overflow.

        Frequencies are given in OS ticks (1 ms)
    */

    // Configure timer 2
    // Targeting an interrupt every 1 ms
    if (RCC->CFGR & RCC_CFGR_PPRE1_2) freq *= 2; // RM0394 pg 188 (timer clock doubles if apb1 prescaler != 0)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN;
    TIM7->PSC = (freq / 1000000) - 1;
    TIM7->ARR = ARR_SET;
    TIM7->CR1 &= ~(TIM_CR1_DIR);
    TIM7->DIER |= TIM_DIER_UIE;

    // Default all values
    memsetu((uint8_t*) &sched, 0, sizeof(sched));
    sched.of = freq;
}

// @funcname: schedStart()
//
// @brief: Starts tasks. Will never return
void schedStart()
{
    TIM7->CR1     |= TIM_CR1_CEN;
    NVIC->ISER[1] |= 1 << (TIM7_IRQn - 32);
    IWDG->KR      =  0xCCCC;     
    IWDG->KR      =  0x5555;
    IWDG->PR      |= 2;
    IWDG->RLR     =  20;
    sched.running =  1;

    while ((IWDG->SR & 0b111) != 0);

    IWDG->KR = 0xAAAA;

    schedLoop();
}

// @funcname: schedPause()
//
// @brief: Stops scheduling and allows schedStart() to return
//         Does not need re-initialization after calling
//
//  @note: This pause must be *temporary*. If you do not reset
//         the watchdog every 3ms, or restart the scheduler
//         within 3ms, your MCU will reset
void schedPause()
{
    TIM7->CR1 &= ~TIM_CR1_CEN;
    NVIC->ISER[1] &= ~(1 << (TIM7_IRQn - 32));
    sched.running = 0;
    sched.run_next = 1;
}

// @funcname: waitMicros()
//
// @brief: Waits for set time in microseconds,
//         but never for more than 100μs at a time
//
// @param: time: time to wait in μs
//
// @note: Think long and hard about the use of
//        this function before you slap it everywhere
void waitMicros(uint8_t time)
{
    if (time > 100) time = 100;

    // ARR is 1k, so no cast issues
    uint16_t entry_time = TIM7->CNT;
    int16_t  exit_time = (int16_t) entry_time - time;

    if (exit_time < 0)
    {
        exit_time += 1000;
        while (TIM7->CNT < entry_time);
    }

    while (TIM7->CNT > exit_time);
}

// @funcname: checkPreflightSwap()
//
// @brief: Checks if we're clear of preflight,
//         and if we're able to swap to primary task pool
static void checkPreflightEnd()
{
    int32_t new_time;

    if (!sched.anim_min_time)
    {
        sched.anim_complete = 1;
    }
    else
    {
        --sched.anim_min_time;
    }
}

// @funcname: schedLoop()
//
// @brief: Main loop that'll run each task
static void schedLoop()
{
    // Locals
    uint8_t i;

    while (sched.running == 1)
    {
        // Prep iteration
        sched.run_next = 0;
        IWDG->KR = 0xAAAA;

        sched.fg_time.tick_entry = sched.os_ticks;
        sched.fg_time.cnt_entry = TIM7->CNT;

        // Execute tasks
        if (sched.preflight_required && (!sched.anim_complete || !sched.preflight_complete))
        {
            if (sched.os_ticks % sched.anim_time == 0)
            {
                sched.anim();
            }

            sched.preflight();
            checkPreflightEnd();
        }
        else
        {
            for (i = 0; i < sched.fg_count; i++)
            {
                if ((sched.os_ticks - sched.ind_fg_time[i].tick_entry) > sched.task_time[i])
                {
                    sched.ind_fg_time[i].tick_entry = sched.os_ticks;
                    sched.ind_fg_time[i].cnt_entry = TIM7->CNT;
                    (*sched.task_pointer[i])();
                    sched.ind_fg_time[i].tick_exit = sched.os_ticks;
                    sched.ind_fg_time[i].cnt_exit = TIM7->CNT;
                }
            }
        }

        sched.fg_time.tick_exit = sched.os_ticks;
        sched.fg_time.cnt_exit = TIM7->CNT;

        // Check if we missed timing requirements
        if (sched.run_next == 1)
        {
            ++sched.skips;
        }

        schedBg();

        calcTime(&sched.fg_time, 1, E_FG_MISS);
        calcTime(&sched.bg_time, 1, E_BG_MISS);
        calcTime(&sched.ind_fg_time, sched.fg_count, E_IND_FG_MISS);
    }
}

// @funcname: schedBg()
//
// @brief: Background loop running when nothing else is
static void schedBg()
{
    // Locals
    uint8_t i;

    sched.bg_time.tick_entry = sched.os_ticks;
    sched.bg_time.cnt_entry = TIM7->CNT;

    while (1)
    {
        // Check if we should break
        if (sched.run_next == 1)
        {
            sched.bg_time.tick_exit = sched.os_ticks;
            sched.bg_time.cnt_exit = TIM7->CNT;

            return;
        }

        if (!(sched.preflight_required && (!sched.anim_complete || !sched.preflight_complete)))
        {
            // Execute background tasks
            for (i = 0; i < sched.bg_count; i++)
            {
                (*sched.bg_pointer[i])();
            }
        }
    }
}

static void updateTime(cpu_time_t* time)
{
    uint32_t delta;

    delta = time->cnt_exit - time->cnt_entry;
    time->cpu_use = (((float) delta) / ARR_SET) * 100;

    if (time->cpu_use > 100)
    {
        time->cpu_use = 100;
    }

    time->max_cpu_use = (time->cpu_use >  time->max_cpu_use) ? time->cpu_use : time->max_cpu_use;
}

static void calcTime(cpu_time_t* time, uint8_t count, int type)
{
    uint8_t error;
    uint8_t e_cnt;
    uint8_t i;

    e_cnt = 0;

    for (i = 0; i < count; i++)
    {
        error = (type != E_BG_MISS) ? time[i].tick_entry != time[i].tick_exit : time[i].tick_exit - time[i].tick_entry > 1;

        if (error)
        {
            time[i].cpu_use = 100;
            time[i].has_missed = 1;
            ++e_cnt;
        }
        else
        {
            updateTime(&time[i]);
        }
    }

    if (e_cnt)
    {
        sched.error |= 1U << type;
    }
    else
    {
        sched.error &= ~(1U << type);
    }
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
    size_t i;

    for (i = 0; i < size; i++)
    {
        ptr[i] = val;
    }
}

// @funcname: TIM7_IRQHandler()
//
// @brief: Timer 7 IRQ. Increments OS ticks and unblocks loop
void TIM7_IRQHandler()
{

	TIM7->SR &= ~TIM_SR_UIF;
    ++sched.os_ticks;

    sched.run_next = 1;
}