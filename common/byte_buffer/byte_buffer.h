#include <stdint.h>
#include <string.h>

#define define_byte_buffer(NAME, MAX_SIZE) \
    uint8_t NAME##_data[(MAX_SIZE)]; \
    byte_buffer_t NAME = { \
        .data    = NAME##_data, \
        .length  = 0, \
        .max_len = (MAX_SIZE), \
    }

typedef struct {
    uint8_t *data;
    size_t length;
    size_t max_len;
} byte_buffer_t;

void buffer_clear(byte_buffer_t *buf);
bool buffer_append(byte_buffer_t *buf, const void *data, size_t length);