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
#include "common/freertos/freertos.h"

// ! assumes the messages are SID for optimal packing
typedef struct {
    uint32_t ticks_ms; // ms timestamp of reception
    uint16_t msg_id;   // message id 
    uint8_t bus_id;    // bus the message was rx'd on
    uint8_t dlc;       // data length code
    uint64_t payload;  // message data
} timestamped_frame_t;
static_assert(sizeof(timestamped_frame_t) == 16,
    "timestamped_frame_t must be 16 bytes for optimal packing");

// todo determine appropriate values based on testing
static constexpr size_t SPMC_NUM_FRAMES = 256;
static constexpr size_t MIN_WRITE_FRAMES = 32;
static_assert((SPMC_NUM_FRAMES % MIN_WRITE_FRAMES) == 0,
    "SPMC_NUM_FRAMES must be a multiple of MIN_WRITE_FRAMES to prevent DMA wraparound issues");

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

void SPMC_init(SPMC_t *spmc, TaskHandle_t master_task, TaskHandle_t follower_task);
int SPMC_enqueue_ISR(SPMC_t *spmc, timestamped_frame_t *incoming_frame);
size_t SPMC_master_peek_batch(SPMC_t *spmc, timestamped_frame_t **first_item);
size_t SPMC_master_get_total(SPMC_t *spmc, timestamped_frame_t **first_item);
void SPMC_master_commit(SPMC_t *spmc, size_t num_consumed);
int SPMC_follower_pop(SPMC_t *spmc, timestamped_frame_t **out, uint32_t *consecutive_items);

#endif // SPMC_H