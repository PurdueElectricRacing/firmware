#include "byte_buffer.h"

define_byte_buffer(testing_buf, 32);

const uint8_t WRCFGA[2] = { 0x00, 0x01 };
const char *testing = "stringstring";

void main() {
    char *localstring = "hello";
    buffer_append(&testing_buf, WRCFGA, 2);
    buffer_append(&testing_buf, testing, 12);
    buffer_append(&testing_buf, localstring, 5);
}