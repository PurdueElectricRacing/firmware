#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "common/freertos/freertos.h"
#define STM32F407xx

// design requirements
// Single producer (CAN_IRQs w same priority)
// Multiple Consumer (SD, ETH)
// the SD head must never miss a message (or else mark overflow)
// the ETH head does "best effort streaming"
// high throughput, lock free

/*
Assumptions
1. There is effectively a single producer
2. The master tail is managed by a higher priority task than the follower
3. the master tail will enter blocked state between getting the batch and committing the tail
4. we do not care if the follower "drops" data
*/

// ! assumes the messages are SID for optimal packing
typedef struct { // 16 bytes
    uint32_t ticks_ms; // ms timestamp of reception
    uint16_t msg_id;   // message id 
    uint8_t bus_id;    // bus the message was rx'd on
    uint8_t dlc;       // data length code
    uint64_t payload;   // message data
} timestamped_frame_t;

// todo determine appropriate values
/* DMA can only access contiguous items,
    size the writes to be a multiple of num_frames to prevent wrap around issues 
    maybe commit the last write right when power is lost?? idk
*/
static constexpr size_t SPMC_NUM_FRAMES = 256;
static constexpr size_t MIN_WRITE_FRAMES = 32;

typedef struct {
    timestamped_frame_t data[SPMC_NUM_FRAMES];
    volatile size_t head;
    volatile size_t master_tail; // SD tail
    TaskHandle_t master_task;
    volatile size_t follower_tail; // best-effort ETH tail
    TaskHandle_t follower_task;

    // metadata
    uint32_t overflows;
} SPMC_t;
SPMC_t queue;


void SPMC_init(
    SPMC_t *spmc,
    TaskHandle_t master_task,
    TaskHandle_t follower_task
) {
    memset(spmc, 0, sizeof(SPMC_t));
    spmc->master_task = master_task;
    spmc->follower_task = follower_task;
    spmc->head = 0;
    spmc->master_tail = 0;
    spmc->follower_tail = 0;
}

// ! note: the two producer ISRs must have the same priority to prevent interruption in the middle of a write
int SPMC_enqueue_ISR(
    SPMC_t *spmc,
    timestamped_frame_t *incoming_frame
) {
    // calc next head and account for wraparound
    size_t next_head = spmc->head + 1;
    if (next_head == SPMC_NUM_FRAMES) {
        next_head = 0;
    }

    if (next_head == spmc->master_tail) {
        spmc->overflows++;
        return -1;
    }

    // write the frame into the buffer
    spmc->data[spmc->head] = *incoming_frame;

    // commit next head only after the write is completed
    spmc->head = next_head;

    size_t num_frames = (spmc->head + SPMC_NUM_FRAMES - spmc->master_tail) % SPMC_NUM_FRAMES;
    if (num_frames >= MIN_WRITE_FRAMES) {
        // Wake the master thread
        vTaskNotifyGiveFromISR(spmc->master_task, NULL);
        return 0;
    }
}

// get a contiguous batch for DMA transfer without actually committing the master tail yet
// returns the number of items?
size_t SPMC_master_peek_batch(
    SPMC_t *spmc,
    timestamped_frame_t **first_item
) {
    const size_t head = spmc->head;
    const size_t tail = spmc->master_tail;

    if (head == tail) {
        *first_item = NULL;
        return 0;
    }

    *first_item = &spmc->data[tail];
    
    if (head > tail) {
        return head - tail;
    } else {
        return SPMC_NUM_FRAMES - tail;
    }
}

/*
    this function must be kept extremely short to prevent
    overflowing the CAN peripheral and missing data
*/ 
void SPMC_master_commit(SPMC_t *spmc, size_t num_consumed) {
    // defer producer IRQs to prevent a .. race condition?
    NVIC_DisableIRQ(CAN1_RX0_IRQn);
    // todo disable the other IRQs too
    const size_t next_tail = (spmc->master_tail + num_consumed) % SPMC_NUM_FRAMES;
    spmc->master_tail = next_tail;
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

/*
    get a single frame we dont care how fast it is or how out of date the data is
*/ 
int SPMC_follower_pop(SPMC_t *spmc, timestamped_frame_t *out) {
    if (spmc->follower_tail == spmc->head) {
        return -1;
    }

    *out = spmc->data[spmc->follower_tail];

    // todo "catchup" the tail if it's too far behind?

    size_t next = spmc->follower_tail + 1;
    if (next == SPMC_NUM_FRAMES) next = 0;
    spmc->follower_tail = next;

    return 0;
}

