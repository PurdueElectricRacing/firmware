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

// Stack size defs
#define STACK_256  (256)
#define STACK_512  (512)
#define STACK_1024 (1024)
#define STACK_2048 (2048)
#define STACK_4096 (4096)

void rtosWrapper(void *arg);

/**
 * @brief Scaffolds the static memory for a FreeRTOS task.
 *
 * @param NAME: The function name of the task.
 * @param PERIOD_MS: Task period in milliseconds.
 * @param PRIORITY: CMSIS-RTOS2 priority (e.g., osPriorityNormal).
 * @param STACK_SIZE: Stack size in bytes.
 */
#define DEFINE_TASK(NAME, PERIOD_MS, PRIORITY, STACK_SIZE)                     \
    static_assert(                                                             \
        (STACK_SIZE) % sizeof(StackType_t) == 0,                               \
        "Stack size must be a multiple of StackType_t"                         \
    );                                                                         \
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
 * @brief Initializes and starts the defined task.
 */
#define START_TASK(NAME)                                                       \
    (NAME##_wrapper.handle = osThreadNew(rtosWrapper, &NAME##_wrapper, &NAME##_wrapper.attrs))

/**
 * @brief Retrieves the FreeRTOS task handle for the defined task.
 */
#define getTaskHandle(NAME) (NAME##_wrapper.handle)

/**
 * @brief Scaffolds the static memory for a FreeRTOS queue.
 */
#define DEFINE_QUEUE(NAME, ITEM, COUNT)                                        \
    QueueHandle_t NAME;                                                        \
    static StaticQueue_t xStaticQueue_##NAME;                                  \
    static uint8_t       ucQueueStorageArea_##NAME[sizeof(ITEM) * (COUNT)];

/**
 * @brief Initializes the defined static queue.
 */
#define INIT_QUEUE(NAME, ITEM, COUNT)                                          \
    (NAME = xQueueCreateStatic((COUNT), sizeof(ITEM),                          \
                               ucQueueStorageArea_##NAME, &xStaticQueue_##NAME))

/**
 * @brief Scaffolds the static memory for a mutex.
 */
#define DEFINE_SEMAPHORE(NAME)                                                 \
    SemaphoreHandle_t NAME;                                                    \
    static StaticSemaphore_t xStaticSemaphore_##NAME;

// Aliases for clarity
#define DEFINE_MUTEX(NAME) DEFINE_SEMAPHORE(NAME)
#define DEFINE_COUNTING_SEMAPHORE(NAME) DEFINE_SEMAPHORE(NAME)
#define DEFINE_BINARY_SEMAPHORE(NAME) DEFINE_SEMAPHORE(NAME)

/**
 * @brief Initializes the defined mutex.
 */
#define INIT_MUTEX(NAME)                                                       \
    (NAME = xSemaphoreCreateMutexStatic(&(xStaticSemaphore_##NAME)))

/**
 * @brief Initializes the defined static counting semaphore.
 * @note initial count is set to max count (full by default)
 */
#define INIT_COUNTING_SEMAPHORE(NAME, MAX_COUNT)                               \
    (NAME = xSemaphoreCreateCountingStatic((MAX_COUNT), (MAX_COUNT), &(xStaticSemaphore_##NAME)))
    
/**
 * @brief Initializes the defined static binary semaphore.
 */
#define INIT_BINARY_SEMAPHORE(NAME)                                            \
    (NAME = xSemaphoreCreateBinaryStatic(&(xStaticSemaphore_##NAME)))


// Timing helper macros
#define getTick()   xTaskGetTickCount()
#define getMS() (getTick() * portTICK_PERIOD_MS)
#define mDelay(ms) (osDelay(pdMS_TO_TICKS((ms))))

#endif // __COMMON_FREERTOS_H__