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
    void (*taskFunction)(void);
    uint32_t       period_ms;
    osThreadAttr_t attrs;
    osThreadId_t   handle;
} ThreadWrapper_t;

void rtosWrapper(void *arg);

/**
 * DEFINE_TASK: Scaffolds the static memory for a FreeRTOS task.
 * @param NAME: The function name of the task.
 * @param PERIOD_MS: Task period in milliseconds.
 * @param PRIORITY: CMSIS-RTOS2 priority (e.g., osPriorityNormal).
 * @param STACK_SIZE: Stack size in bytes.
 */
#define DEFINE_TASK(NAME, PERIOD_MS, PRIORITY, STACK_SIZE)                     \
    static StaticTask_t task_cb_##NAME;                                        \
    static StackType_t  task_stack_##NAME[(STACK_SIZE) / sizeof(StackType_t)]; \
    ThreadWrapper_t     NAME##_wrapper = {                                     \
        .taskFunction = (void (*)(void))NAME,                                  \
        .period_ms    = (PERIOD_MS),                                           \
        .attrs = {                                                             \
            .name       = #NAME,                                               \
            .attr_bits  = osThreadDetached,                                    \
            .cb_mem     = &task_cb_##NAME,                                     \
            .cb_size    = sizeof(StaticTask_t),                                \
            .stack_mem  = task_stack_##NAME,                                   \
            .stack_size = sizeof(task_stack_##NAME),                           \
            .priority   = (osPriority_t)(PRIORITY),                            \
        }                                                                      \
    }

/**
 * START_TASK: Initializes and starts the defined task.
 */
#define START_TASK(NAME)                                                       \
    (NAME##_wrapper.handle = osThreadNew(rtosWrapper, &NAME##_wrapper, &NAME##_wrapper.attrs))

#define getTaskHandle(NAME) (NAME##_wrapper.handle)

/**
 * DEFINE_STATIC_QUEUE: Scaffolds the static memory for a FreeRTOS queue.
 */
#define DEFINE_STATIC_QUEUE(NAME, ITEM, COUNT)                                 \
    QueueHandle_t NAME;                                                        \
    static StaticQueue_t xStaticQueue_##NAME;                                  \
    static uint8_t       ucQueueStorageArea_##NAME[sizeof(ITEM) * (COUNT)];    \
    QueueHandle_t        NAME

/**
 * CREATE_STATIC_QUEUE: Initializes the defined static queue.
 */
#define CREATE_STATIC_QUEUE(NAME, ITEM, COUNT)                                 \
    (NAME = xQueueCreateStatic((COUNT), sizeof(ITEM),                          \
                               ucQueueStorageArea_##NAME, &xStaticQueue_##NAME))

/**
 * DEFINE_STATIC_SEMAPHORE: Scaffolds the static memory for a mutex.
 */
#define DEFINE_STATIC_SEMAPHORE(NAME)                                          \
    SemaphoreHandle_t NAME;                                                    \
    static StaticSemaphore_t xStaticSemaphore_##NAME;                          \
    SemaphoreHandle_t        NAME

/**
 * CREATE_STATIC_SEMAPHORE: Initializes the defined static semaphore.
 */
#define CREATE_STATIC_SEMAPHORE(NAME)                                          \
    (NAME = xSemaphoreCreateMutexStatic(&(xStaticSemaphore_##NAME)))


#define getTick()   xTaskGetTickCount()
#define getTickms() pdMS_TO_TICKS(getTick())

#define mDelay(ms) (osDelay(pdMS_TO_TICKS((ms))))

// ! Legacy support for old macro names for now
#define defineThread(T, D, P)         DEFINE_TASK(T, D, P, 1024)
#define defineThreadStack(T, D, P, S) DEFINE_TASK(T, D, P, S)
#define createThread(NAME)            START_TASK(NAME)
#define defineStaticQueue(N, I, C)    DEFINE_STATIC_QUEUE(N, I, C)
#define createStaticQueue(N, I, C)    CREATE_STATIC_QUEUE(N, I, C)
#define defineStaticSemaphore(N)      DEFINE_STATIC_SEMAPHORE(N)
#define createStaticSemaphore(N)      CREATE_STATIC_SEMAPHORE(N)

#endif // __COMMON_FREERTOS_H__
