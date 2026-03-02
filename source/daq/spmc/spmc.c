/**
 * @file spmc.c
 * @brief Custom SPMC designed for high throughput handling of CAN message streams.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Shriya Balu (balu@purdue.edu)
 */

#include "spmc.h"
#include "stm32f407xx.h"

// todo maybe commit the last write right when power is lost?

static constexpr uint32_t CAN_RX_IRQ_PRIO = 6;  // highest RTOS priority
static constexpr uint32_t CAN_SCE_IRQ_PRIO = 10;
static_assert(
    CAN_RX_IRQ_PRIO < CAN_SCE_IRQ_PRIO,
    "RX IRQs should have higher priority than SCE IRQs"
);

/**
 * @brief Initializes the SPMC instance and configures CAN RX interrupts.
 *
 * @param spmc Pointer to the SPMC instance to initialize.
 */
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


/**
 * @brief Enqueues a received CAN message into the SPMC buffer from an ISR context.
 * ! the two producer ISRs must have the same priority to prevent preemption in the middle of a write
 * 
 * @param spmc Pointer to the SPMC instance.
 * @param incoming_frame Pointer to the timestamped_frame_t containing the received CAN message.
 */
int SPMC_enqueue_from_ISR(SPMC_t *spmc, timestamped_frame_t *incoming_frame) {
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
    __DMB(); // do not switch the order of the data write and head update
    spmc->head = next_head;

    __DSB(); // halt the cpu until write is finished

    return 0;
}

/**
 * @brief Peeks at the next batch of frames available for processing by the master (SD logging) without committing the tail.
 * Intended to be used with a DMA operation.
 * 
 * @param spmc Pointer to the SPMC instance.
 * @param first_item Output pointer that will point to the first item in the batch if available, or NULL if no items are available.
 * @param total_unread Output pointer that will be set to the total number of unread frames available in the buffer.
 *
 * @return The number of contiguous items available in the batch.
 */
size_t SPMC_master_peek_all(SPMC_t *spmc, timestamped_frame_t **first_item, size_t *total_unread) {
    const size_t head = spmc->head;
    __DMB(); // ensure data visibility of head before reading tail
    const size_t tail = spmc->master_tail;

    // empty case
    if (head == tail) {
        *first_item = nullptr;
        *total_unread = 0;
        return 0;
    }

    __DMB(); // dont read the buffer before tail/head are ready
    *first_item = &spmc->data[tail];
    
    // contiguous case
    if (head > tail) {
        *total_unread = head - tail;
        return *total_unread;
    }

    // wraparound case
    *total_unread = (SPMC_NUM_FRAMES - tail) + head;
    return (SPMC_NUM_FRAMES - tail);
}

/**
 * @brief Commits the specified number of consumed frames by advancing the master tail pointer.
 * 
 * @param spmc Pointer to the SPMC instance.
 * @param num_consumed The number of frames to commit.
 */
void SPMC_master_commit_tail(SPMC_t *spmc, size_t num_consumed) {
    // mask the CAN RX IRQs
    uint32_t basepri = __get_BASEPRI();
    __set_BASEPRI(CAN_RX_IRQ_PRIO << (8 - __NVIC_PRIO_BITS));

    const size_t next_tail = (spmc->master_tail + num_consumed) % SPMC_NUM_FRAMES;
    spmc->master_tail = next_tail;
    __DSB();

    __set_BASEPRI(basepri);
}

/**
 * @brief Pops a single frame for the follower (Ethernet transmission) and advances the follower tail pointer.
 * todo: this function is incomplete
 * @param spmc Pointer to the SPMC instance.
 * @param out Output pointer to the popped frame.
 * @param consecutive_items Output pointer to the number of consecutive frames available.
 * @return 0 on success, -1 if no frames are available.
 */
int SPMC_follower_pop(SPMC_t *spmc, timestamped_frame_t **out, uint32_t *consecutive_items) {
    const size_t head = spmc->head;
    __DMB(); // ensure data visibility of head before reading tail
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

