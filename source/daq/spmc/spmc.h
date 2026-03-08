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

typedef struct {
    uint32_t ticks_ms; // ms timestamp of reception
    uint32_t identity; // [1 bit bus ID] [1 bit isExtID] [1 bit reserved] [29 bits CAN ID]
    uint64_t payload;  // message data
} timestamped_frame_t;

static_assert(
    sizeof(timestamped_frame_t) == 16,
    "timestamped_frame_t must be 16 bytes for optimal access patterns"
);

// todo tune values based on testing
static constexpr size_t SPMC_CAPACITY = 512;
static constexpr size_t SD_WRITE_THRESHOLD = 32;
static_assert(
    SPMC_CAPACITY % SD_WRITE_THRESHOLD == 0,
    "the SPMC capacity must be a multiple of SD_WRITE_THRESHOLD "
    "to prevent DMA wraparound issues"
);
// ! allocate one extra frame to distinguish full vs empty conditions
static constexpr size_t SPMC_ALLOCATED_CAPACITY = SPMC_CAPACITY + 1;

typedef struct {
    timestamped_frame_t data[SPMC_ALLOCATED_CAPACITY];
    volatile size_t head;
    volatile size_t master_tail; // SD tail
    volatile size_t follower_tail; // best-effort ETH tail
    volatile uint32_t overflows;
} SPMC_t;

void SPMC_init(SPMC_t *spmc);
int SPMC_enqueue_from_ISR(SPMC_t *spmc, timestamped_frame_t *incoming_frame);
size_t SPMC_master_peek_all(SPMC_t *spmc, timestamped_frame_t **first_item, size_t *total_unread);
void SPMC_master_commit_tail(SPMC_t *spmc, size_t num_consumed);
int SPMC_follower_pop(SPMC_t *spmc, timestamped_frame_t **out, uint32_t *consecutive_items);

#endif // SPMC_H