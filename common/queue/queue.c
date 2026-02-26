/**
 * @file queue.c
 * @brief Simple static queue implementation.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "queue.h"
#include <string.h>

queue_status_t queue_push(queue_t *q, void *tx) {
    size_t next_tail = (q->tail + q->item_size) % q->max_size;

    if (next_tail == q->head) {
        return QUEUE_FULL;
    }

    memcpy(q->data + q->tail, tx, q->item_size);
    q->tail = next_tail;
    return QUEUE_SUCCESS;
}

queue_status_t queue_pop(queue_t *q, void *rx) {
    if (q->head == q->tail) {
        return QUEUE_EMPTY;
    }

    memcpy(rx, q->data + q->head, q->item_size);
    q->head = (q->head + q->item_size) % q->max_size;
    return QUEUE_SUCCESS;
}
