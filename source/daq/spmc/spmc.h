#ifndef SPMC_H
#define SPMC_H

/**
 * @file spmc.h
 * @brief Custom SPMC designed for high throughput handling of CAN message streams.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stddef.h>
#include <stdint.h>
#include "timestamped_frame.h"

// todo tune values based on testing
static constexpr size_t SPMC_CHUNK_NUM_FRAMES = 128;
static constexpr size_t SPMC_CHUNK_CAPACITY = 12;

static constexpr size_t SPMC_FRAME_CAPACITY = SPMC_CHUNK_NUM_FRAMES * SPMC_CHUNK_CAPACITY;
static_assert(
    SPMC_FRAME_CAPACITY % SPMC_CHUNK_NUM_FRAMES == 0,
    "the SPMC capacity must be a multiple of SPMC_CHUNK_NUM_FRAMES "
    "to prevent DMA wraparound issues and fragmentation"
);

typedef struct {
    timestamped_frame_t data[SPMC_FRAME_CAPACITY];
    volatile size_t head;          // shared head
    volatile size_t master_tail;   // SD tail
    volatile size_t follower_tail; // best-effort ETH tail

    // stats
    volatile uint32_t overflows;      // items dropped due to full buffer
    volatile uint32_t follower_drops; // follower items lost due to lagging too far behind
    volatile bool is_full;
} SPMC_t;

static_assert(
    sizeof(size_t) == 4,
    "32-bit loads and stores are atomic, this is required for the lock-free design to work"
);

void SPMC_init(SPMC_t *spmc);
bool SPMC_enqueue_from_ISR(SPMC_t *spmc, timestamped_frame_t *incoming_frame);

bool SPMC_master_peek_chunk(SPMC_t *spmc, timestamped_frame_t **first_item);
void SPMC_master_advance_tail(SPMC_t *spmc);

size_t SPMC_follower_peek_chunks(SPMC_t *spmc, timestamped_frame_t **first_item);
void SPMC_follower_advance_tail(SPMC_t *spmc, size_t chunks_consumed);

#endif // SPMC_H