#include "byte_buffer.h"

void buffer_clear(byte_buffer_t *buf) {
    buf->length = 0;
}

bool buffer_append(byte_buffer_t *buf, const void *data, size_t length) {
    if (buf->length + length > buf->max_len) {
        return false;
    }

    uint8_t *buf_end_addr = buf->data + buf->length;
    memcpy(buf_end_addr, data, length);
    buf->length += length;
    return true;
}