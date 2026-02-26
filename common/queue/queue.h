#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t *data;
    size_t head;
    size_t tail;
    size_t item_size;
    size_t max_size;
} queue_t;

typedef enum {
    QUEUE_SUCCESS = 0,
    QUEUE_FULL    = 1,
    QUEUE_EMPTY   = 2
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

queue_status_t queue_push(queue_t *q, void *tx);
queue_status_t queue_pop(queue_t *q, void *rx);

#endif // QUEUE_H