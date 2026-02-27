## Queue
Standard FIFO queue implementation.
- Circular buffer allocated at compile time.

API:
- `QUEUE_INIT(name, item_size, num_items)`: allocates a queue in global memory
- `queue_push(queue_t *q, void *tx)`: pushes an item to the back of the queue
- `queue_pop(queue_t *q, void *rx)`: pops an item from the front of the queue

> [!CAUTION]
> The queue is strictly single producer, single-consumer.
> It is not thread-safe and does not use any synchronization primitives.
