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
    uint32_t identity;   // [1 bit bus ID] [1 bit isExtID] [1 bit reserved] [29 bits CAN ID]
    uint64_t payload;  // message data
} timestamped_frame_t;

static_assert(
    sizeof(timestamped_frame_t) == 16,
    "timestamped_frame_t must be 16 bytes for optimal access patterns"
);

// todo determine appropriate values based on testing
// +1 to distinguish full vs empty
static constexpr size_t SPMC_NUM_FRAMES = 256 + 1;
static constexpr size_t MIN_WRITE_FRAMES = 32;
static_assert(
    (SPMC_NUM_FRAMES -1 ) % MIN_WRITE_FRAMES == 0,
    "the usable capacity must be a multiple of MIN_WRITE_FRAMES "
    "to prevent DMA wraparound issues"
);

typedef struct {
    timestamped_frame_t data[SPMC_NUM_FRAMES];
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