/**
 * @file strbuf.c
 * @brief Fixed-size string builder.
 * Useful for building large CMD strings before transmission.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef STRBUF_H
#define STRBUF_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t *data;
    size_t length;
    size_t max_len;
} strbuf_t;

void strbuf_clear(strbuf_t *sb);
size_t strbuf_append(strbuf_t *sb, const void *data, size_t length);

#define allocate_strbuf(NAME, MAX_SIZE) \
    uint8_t NAME##_data[(MAX_SIZE)]; \
    strbuf_t NAME = { \
        .data    = NAME##_data, \
        .length  = 0, \
        .max_len = (MAX_SIZE), \
    }

#endif // STRBUF_H
