#include "queue.h"

// @funcname: qConstruct
//
// @brief: Initializes queue for specific item type
//
// @param: q: Handle for queue
// @param: size: Size of each item in queue in bytes
void qConstruct(q_handle_t* q, uint8_t size)
{
    q->item_count = 0;                                          // Initialize number of items to 0
    q->size = size;                                             // Initialize size to sizeof(struct)
    q->start = q->buffer;                                       // Set start pointer to beginning of buffer
    q->current = q->buffer;                                     // Set current pointer to beginning of buffer
    q->max_items = (uint8_t) (MEM_SIZE / (float) size);         // Calculate maximum number of items
}

// @funcname: qIsFull
//
// @brief: Returns 1 if the queue is full, 0 else
//
// @param: q: Handle for queue
//
// @return: 1 if queue is full, 0 else
uint8_t qIsFull(q_handle_t* q)
{
    return q->item_count == q->max_items ? 1 : 0;               // Return 1 if full, 0 else
}

// @funcname: qSendToBack
//
// @brief: Adds an item to the queue (enqueue)
//
// @param: q: Handle for queue
// @param: item: Item to be added to queue
//
// @return: SUCCESS if value is added, FAILURE if queue is full
success_t qSendToBack(q_handle_t* q, const void* item)
{
    if (q->item_count == q->max_items)                          // Ensure that we have a location for the item
    {
        return FAILURE_G;                                       // We don't, so let the user know it failed
    }

    if ((q->current - q->buffer) == q->size * q->max_items)     // Check if the current pointer location is at the end of the buffer
    {
        q->current = q->buffer;                                 // Reset the buffer location
    }

    memcpy(q->current, item, q->size);                          // Copy item into queue at current location
    q->current = q->current + q->size;                          // Increment pointer by number of bytes corresponding to item size
    ++q->item_count;                                            // Increment number of items in queue

    return SUCCESS_G;
}

// @funcname: qReceive
//
// @brief: Removes an item to the queue (dequeue)
//
// @param: q: Handle for queue
// @param: rx_buf: Location for the item to be "moved" to
//
// @return: SUCCESS if value is removed, FAILURE if queue is empty
success_t qReceive(q_handle_t* q, void* rx_buf)
{
    if (q->item_count == 0)                                     // Ensure that we actually have an item in the queue
    {
        return FAILURE_G;                                       // We don't, so let the user know it failed
    }

    if ((q->start - q->buffer) == q->size * q->max_items)       // Check if the start pointer location is at the end of the buffer
    {
        q->start = q->buffer;                                   // Reset the buffer location
    }

    memcpy(rx_buf, q->start, q->size);                          // Copy item from queue into buffer
    q->start = q->start + q->size;                              // Increment pointer by number of bytes corresponding to item size
    --q->item_count;                                            // Decrement number of items in queue

    return SUCCESS_G;
}