/**
 * @file buffer.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Buffer for one producer, multiple consumers
 * @version 0.1
 * @date 2024-02-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool active;
    uint32_t _tail; // Element number to read
} b_tail_t;

typedef struct
{
    volatile uint8_t* buffer; //!< Buffer of data
    b_tail_t* tails; //!< Array of tails that are consuming
    uint8_t num_tails; //!< Number of tails
    volatile uint32_t _head; //!< Element number to write
    uint32_t _min_tail; //!< Oldest element number to be read
    bool _min_tail_active; //!< At least one of the tails are active
    uint32_t _item_size; //!< Size of buffer in bytes
    uint32_t _max_items; //!< Maximum number of items that can be stored (only ever max - 1)
    // Logging
    uint32_t overflows; //!< Number of times the buffer was out of room for writing
    uint32_t underflows; //!< Number of times bCommitRead failed
    uint32_t max_level; //!< Max fill level
} b_handle_t;

void bConstruct(b_handle_t* b, uint32_t item_size, uint32_t buff_size);
int8_t bGetHeadForWrite(b_handle_t* b, void** tx_loc, uint32_t* contiguous_items);
int8_t bCommitWrite(b_handle_t* b, uint32_t items_written);
int8_t bSendToBack(b_handle_t* b, const void* data);
int8_t bGetTailForRead(b_handle_t* b, uint8_t t_idx, void** rx_loc, uint32_t* contiguous_items);
int8_t bCommitRead(b_handle_t* b, uint8_t t_idx, uint32_t items_read);
uint32_t bGetItemCount(b_handle_t* b, uint8_t t_idx);
uint32_t bGetFillLevel(b_handle_t* b);
void bActivateTail(b_handle_t* b, uint8_t t_idx);
void bDeactivateTail(b_handle_t* b, uint8_t t_idx);

#endif
