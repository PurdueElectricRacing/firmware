#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    uint8_t *data;
    size_t head;
    size_t tail;
    size_t item_size;
    size_t max_size;
} queue_t;

typedef enum {
    QUEUE_OK = 0,
    QUEUE_FULL,
    QUEUE_EMPTY
} queue_status_t;

#define QUEUE_INIT(name, item_size_, num_items_) \
    uint8_t name##_data[(item_size_) * (num_items_)]; \
    queue_t name = { \
        .data = name##_data, \
        .head = 0, \
        .tail = 0, \
        .item_size = (item_size_), \
        .max_size = sizeof(name##_data) \
    }

#endif // QUEUE_H

// todo make this a ring buf
queue_status_t queue_push(queue_t *q, void *tx) {
    if ((q->tail + q->item_size) > q->max_size) {
        return QUEUE_FULL;
    }

    memcpy(q->data + q->tail, tx, q->item_size);
    q->tail += q->item_size;
    return QUEUE_OK;
}

queue_status_t queue_pop(queue_t *q, void *rx) {
    if (q->head == q->tail) {
        return QUEUE_EMPTY;
    }

    memcpy(rx, q->data + q->head, q->item_size);
    q->head += q->item_size;
    return QUEUE_OK;
}