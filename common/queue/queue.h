#ifndef _QUEUE_H_
#define _QUEUE_H_

// Includes
#include "stdint.h"
#include "stdbool.h"

// Generic defines
#ifdef MEM_SMALL
#define MEM_SIZE        1024        // Size of overall static memory allocation (small size)
#elif defined MEM_MED
#define MEM_SIZE        1024        // Size of overall static memory allocation (mid size)
#else
#define MEM_SIZE        1024        // Size of overall static memory allocation (large size)
#endif

// Enumerations
// Comment out if declared elsewhere
typedef enum {
    FAILURE_G,
    SUCCESS_G
} success_t;

// Structs
typedef struct {
    volatile uint8_t  buffer[MEM_SIZE]; //!< Ring buffer for queue storage
    volatile uint32_t head;             //!< Element number of first item
    volatile uint32_t tail;             //!< Element number of last item
    uint32_t size;                      //!< Size of each item 
    uint32_t max_items;                 //!< Maximum number of items in queue based on size (can only ever hold max_items - 1)
} q_handle_t;

// Prototypes
void qConstruct(q_handle_t* q, uint8_t size);
uint8_t qIsFull(q_handle_t* q);
uint8_t qIsEmpty(q_handle_t* q);
success_t qSendToBack(q_handle_t* q, const void* item);
success_t qReceive(q_handle_t* q, void* rx_buf);

#endif