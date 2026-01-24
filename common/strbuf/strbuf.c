/**
 * @file strbuf.c
 * @brief Fixed-size string builder.
 * Useful for building large CMD strings before transmission.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "strbuf.h"
#include <string.h>

/**
 * @brief Clears the buffer by resetting length to 0.
 */
void strbuf_clear(strbuf_t *sb) {
    sb->length = 0;
}

/**
 * @brief Appends raw data to the buffer.
 * @return Num bytes written to the buffer
 */
size_t strbuf_append(strbuf_t *sb, const void *data, size_t length) {
    if (sb->length + length > sb->max_len) {
        return 0;
    }

    uint8_t *buf_end_addr = sb->data + sb->length;
    memcpy(buf_end_addr, data, length);
    sb->length += length;

    return length;
}