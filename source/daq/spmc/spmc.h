#ifndef SPMC_H
#define SPMC_H

/**
 * @file spmc.h
 * @brief Custom SPMC designed for high throughput handling of CAN message streams.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Shriya Balu (balu@purdue.edu)
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "timestamped_frame.h"

typedef enum : int {
    SPMC_OK    = 0,
    SPMC_FULL  = -1,
    SPMC_EMPTY = -2,
} SPMC_status_t;

// todo tune values based on testing
static constexpr size_t SPMC_CAPACITY = 512;
static constexpr size_t SD_WRITE_THRESHOLD = 128;
static_assert(
    SPMC_CAPACITY % SD_WRITE_THRESHOLD == 0,
    "the SPMC capacity must be a multiple of SD_WRITE_THRESHOLD "
    "to prevent DMA wraparound issues"
);
// ! allocate one extra frame to distinguish full vs empty conditions
static constexpr size_t SPMC_ALLOCATED_CAPACITY = SPMC_CAPACITY + 1;

typedef struct {
    timestamped_frame_t data[SPMC_ALLOCATED_CAPACITY];
    volatile size_t head;          // shared head
    volatile size_t master_tail;   // SD tail
    volatile size_t follower_tail; // best-effort ETH tail
    volatile uint32_t overflows;   // items dropped due to full buffer
} SPMC_t;

static_assert(
    sizeof(size_t) == 4,
    "32-bit loads and stores are atomic, this is required for the lock-free design to work"
);

void SPMC_init(SPMC_t *spmc);
SPMC_status_t SPMC_enqueue_from_ISR(SPMC_t *spmc, timestamped_frame_t *incoming_frame);
size_t SPMC_master_peek_all(SPMC_t *spmc, timestamped_frame_t **first_item, size_t *total_unread);
void SPMC_master_commit_tail(SPMC_t *spmc, size_t num_consumed);
SPMC_status_t SPMC_follower_pop(SPMC_t *spmc, timestamped_frame_t **out, uint32_t *consecutive_items);

#endif // SPMC_H