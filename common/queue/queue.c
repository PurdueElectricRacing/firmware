#include "queue.h"

/**
 * @brief Initializes queue for specific item type
 * 
 * @param q Handle for queue
 * @param size Size of each item in queue in bytes
 */
void qConstruct(q_handle_t* q, uint8_t size)
{
    q->size = size;                                             // Initialize size to sizeof(struct)
    q->max_items = (MEM_SIZE / (float) size);                   // Calculate maximum number of items
    q->head = q->tail = 0;
}

/**
 * @brief Returns 1 if the queue is full, 0 else
 * 
 * @param q Handle for queue
 * @return 1 if queue is full, 0 else
 */
uint8_t qIsFull(q_handle_t* q)
{
    uint32_t next_head = (q->head + 1) % q->max_items;
    return next_head == q->tail;
}

/**
 * @brief Returns 1 if the queue is empty, 0 else
 * 
 * @param q Handle for queue
 * @return 1 if queue is empty, 0 else
 */
uint8_t qIsEmpty(q_handle_t* q)
{
    return q->head == q->tail;
}

/**
 * @brief Adds an item to the queue (enqueue)
 * @note  Interrupt safe ONLY if ONE producer and consumer 
 * 
 * @param q Handle for queue
 * @param item Item to be added to queue
 * @return success_t SUCCESS if value is added, FAILURE if queue is full
 */
success_t qSendToBack(q_handle_t* q, const void* item)
{
    uint32_t next_head = (q->head + 1) % q->max_items;          // Calculate next head
    if (next_head == q->tail) return FAILURE_G;                 // No room
    volatile uint8_t* dst = q->buffer + (q->head * q->size);
    for (uint32_t i = 0; i < q->size; ++i) dst[i] = ((uint8_t *)item)[i];
    q->head = next_head;                                        // Update head
    return SUCCESS_G;
}

/**
 * @brief Removes an item to the queue (dequeue)
 * @note  Interrupt safe ONLY if ONE producer and consumer 
 * 
 * @param q Handle for queue
 * @param rx_buf Location for the item to be "moved" to
 * @return success_t SUCCESS if value is removed, FAILURE if queue is empty
 */
success_t qReceive(q_handle_t* q, void* rx_buf)
{
    if (q->tail == q->head) return FAILURE_G;                   // Empty
    volatile uint8_t* src = q->buffer + (q->tail * q->size);
    for (uint32_t i = 0; i < q->size; ++i) ((uint8_t *)rx_buf)[i] = src[i];
    q->tail = (q->tail + 1) % q->max_items;                     // Update tail
    return SUCCESS_G;
}
