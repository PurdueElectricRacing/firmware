/**
 * @file spmc.c
 * @brief Custom SPMC designed for high throughput handling of CAN message streams.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Shriya Balu (balu@purdue.edu)
 */

#include "spmc.h"

// todo maybe commit the last write right when power is lost?? idk

static constexpr uint32_t CAN_RX_IRQ_PRIO = 6;  // highest RTOS priority
static constexpr uint32_t CAN_SCE_IRQ_PRIO = 10;
static_assert(
    CAN_RX_IRQ_PRIO < CAN_SCE_IRQ_PRIO,
    "RX IRQs should have higher priority than SCE IRQs"
);

void SPMC_init(SPMC_t *spmc) {
    memset(spmc, 0, sizeof(SPMC_t));
    spmc->head = 0;
    spmc->master_tail = 0;
    spmc->follower_tail = 0;

    NVIC_SetPriority(CAN1_RX0_IRQn, CAN_RX_IRQ_PRIO);
    NVIC_SetPriority(CAN2_RX0_IRQn, CAN_RX_IRQ_PRIO);
    NVIC_SetPriority(CAN1_SCE_IRQn, CAN_SCE_IRQ_PRIO);
    NVIC_SetPriority(CAN2_SCE_IRQn, CAN_SCE_IRQ_PRIO);

    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    NVIC_EnableIRQ(CAN1_SCE_IRQn);
    NVIC_EnableIRQ(CAN2_RX0_IRQn);
    NVIC_EnableIRQ(CAN2_SCE_IRQn);
}

// ! note: the two producer ISRs must have the same priority to prevent preemption in the middle of a write
int SPMC_enqueue_ISR(
    SPMC_t *spmc,
    timestamped_frame_t *incoming_frame
) {
    // calc next head and account for wraparound
    size_t next_head = spmc->head + 1;
    if (next_head == SPMC_NUM_FRAMES) {
        next_head = 0;
    }

    // ! the frame is dropped if the buffer is full
    if (next_head == spmc->master_tail) {
        spmc->overflows++;
        return -1;
    }

    // write the frame into the buffer
    spmc->data[spmc->head] = *incoming_frame;
    spmc->head = next_head;

    return 0;
}

// get a contiguous batch for DMA transfer without actually committing the master tail yet
// returns the number of contiguous items?
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


size_t SPMC_master_get_total(
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
        return SPMC_NUM_FRAMES - (tail - head);
    }
}

/*
    this function must be kept extremely short to prevent
    overflowing the CAN peripheral and missing data
*/ 
void SPMC_master_commit(SPMC_t *spmc, size_t num_consumed) {
    // todo: instead of heavy-handidly disabling all interrupts, we should only mask the two CAN RX ISRs
    uint32_t primask = __get_PRIMASK();
    __disable_irq(); // ! mask producer IRQs to prevent a race condition
    const size_t next_tail = (spmc->master_tail + num_consumed) % SPMC_NUM_FRAMES;
    spmc->master_tail = next_tail;
    if (!primask) {
        __enable_irq();
    }
}

/*
    get a single frame we dont care how fast it is or how out of date the data is
*/ 
int SPMC_follower_pop(SPMC_t *spmc, timestamped_frame_t **out ,uint32_t *consecutive_items) {
    const size_t head = spmc->head;
    if (spmc->follower_tail == head) {
        return -1;
    }

    *out = &spmc->data[spmc->follower_tail];

    // todo "catchup" the tail if it's too far behind?

    size_t next = spmc->follower_tail + 1;
    if (next == SPMC_NUM_FRAMES) next = 0;
    spmc->follower_tail = next;

    if (head > spmc->follower_tail) {
        *consecutive_items = head - spmc->follower_tail;
    } else if (head < spmc->follower_tail) {
        *consecutive_items = SPMC_NUM_FRAMES - spmc->follower_tail;
    } else {
        *consecutive_items = 0;
    }
    return 0;
}

