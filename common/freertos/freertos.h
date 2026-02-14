#ifndef __COMMON_FREERTOS_H__
#define __COMMON_FREERTOS_H__

// clang-format off
#define myIDENT(x) x
#define myXSTR(x) #x
#define mySTR(x) myXSTR(x)
#define __FREERTOS_PATH(x,y) mySTR(myIDENT(x)y)
#define _FREERTOS_PATH(y) __FREERTOS_PATH(_FREERTOS_DIR, y)

#if defined(STM32F407xx)
#define _FREERTOS_DIR external/STM32CubeF4/Middlewares/Third_Party/FreeRTOS/Source/
#elif defined(STM32G474xx)
#define _FREERTOS_DIR external/STM32CubeG4/Middlewares/Third_Party/FreeRTOS/Source/
#else
#error "Unsupported MCU arch"
#endif

#include _FREERTOS_PATH(include/FreeRTOS.h)
#include _FREERTOS_PATH(CMSIS_RTOS_V2/cmsis_os2.h)

#include _FREERTOS_PATH(include/list.h)
#include _FREERTOS_PATH(include/queue.h)
#include _FREERTOS_PATH(include/semphr.h)
#include _FREERTOS_PATH(include/task.h)
#include _FREERTOS_PATH(include/timers.h)
// clang-format on

#include <stdint.h>

typedef struct {
    void (*taskFunction)();
    uint32_t period;
    osThreadAttr_t attrs;
    osThreadId_t handle;
} ThreadWrapper;

void rtosWrapper(void *);

// Cursed macro
#define threadWrapperName(NAME) (threadWrapper_##NAME)
// TASK: task function name
// PERIOD: period of task in ticks (should be ms)
// PRIORITY: one of osPriorityNormal, osPriorityHigh, etc
// STACK: stack size
#define __defineThread(TASK, PERIOD, PRIORITY, STACK) \
    static StaticTask_t taskWrapperCB_##TASK; \
    static StackType_t taskWrapperStack_##TASK[(STACK) / sizeof(StackType_t)]; \
    ThreadWrapper threadWrapperName(TASK) = { \
        .taskFunction = &(TASK), \
        .period       = (PERIOD), \
        .attrs        = { \
            .priority   = (PRIORITY), \
            .stack_size = (STACK), \
            .name       = "\"" #TASK "\"", \
            .cb_mem     = &taskWrapperCB_##TASK, \
            .cb_size    = sizeof(StaticTask_t), \
            .stack_mem  = taskWrapperStack_##TASK, \
        }};

#define defineThread(T, D, P)         __defineThread(T, D, P, 1024) // TODO calculate stack size
#define defineThreadStack(T, D, P, S) __defineThread(T, D, P, S)

#define __createThread(NAME) \
    threadWrapperName(NAME).handle = \
        osThreadNew(rtosWrapper, &(threadWrapperName(NAME)), &(threadWrapperName(NAME)).attrs);
#define createThread(NAME) __createThread(NAME)

#define getTaskHandle(NAME) threadWrapperName(NAME).handle

// queues
#define defineStaticQueue(NAME, ITEM, COUNT) \
    QueueHandle_t NAME; \
    static StaticQueue_t xStaticQueue_##NAME; \
    uint8_t ucQueueStorageArea_##NAME[sizeof(ITEM) * (COUNT)];

#define createStaticQueue(NAME, ITEM, COUNT) \
    xQueueCreateStatic((COUNT), sizeof(ITEM), ucQueueStorageArea_##NAME, &xStaticQueue_##NAME);

// semaphores
#define defineStaticSemaphore(NAME) \
    SemaphoreHandle_t NAME; \
    static StaticSemaphore_t xStaticSemaphore_##NAME;
#define createStaticSemaphore(NAME) xSemaphoreCreateMutexStatic(&(xStaticSemaphore_##NAME));

#define getTick()   xTaskGetTickCount()
#define getTickms() pdMS_TO_TICKS(getTick())

#define mDelay(ms) (osDelay(pdMS_TO_TICKS((ms))))

#endif // __COMMON_FREERTOS_H__
