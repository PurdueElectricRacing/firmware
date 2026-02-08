## strbuf
The `strbuf` module provides a simple and efficient way to handle "dynamic" strings in C.
Data is still allocated statically, but the module allows you to append data to a buffer while keeping track of the current length and ensuring that you do not exceed the allocated size.

#### Usage Example:
```c
#include "strbuf.h"

#define CMD_LEN 2
const uint8_t WRCFGA[CMD_LEN] = { 0x00, 0x01 };
const char *globalstring = "hello";

// Allocate the buffer with size of 32 bytes
ALLOCATE_STRBUF(test_buf, 32);

int main() {
    char *localstring = "world";

    // You can check the return value to see if the operation was successful
    if (0 == strbuf_append(&test_buf, WRCFGA, CMD_LEN)) {
        return 1; // failure
    }

    // Reset the buffer
    strbuf_clear(&test_buf);

    // Store "Hello World"
    strbuf_append(&test_buf, globalstring, 5);
    strbuf_append(&test_buf, " ", 1);
    strbuf_append(&test_buf, localstring, 5);

    return 0;
}
```