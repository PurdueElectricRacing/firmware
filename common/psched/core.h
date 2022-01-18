#ifndef _CORE_H_
#define _CORE_H_

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
#include "schedule.h"

// Heap sizing
#define SIZE_KB         4
#define HEAP_SIZE       (1024 * SIZE_KB)
#define REG_COUNT       12
#define REG_SIZE        (32 * REG_COUNT)

// Typedefs, enums, and structures
typedef uint8_t heap_t;

typedef struct {
    bool is_bg;
    bool is_running;
    heap_t* stack_loc;
    heap_t* reg_loc;
} tcb_t;

heap_t psched_heap[HEAP_SIZE];

#endif