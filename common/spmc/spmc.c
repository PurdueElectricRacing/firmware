#include <stddef.h>
#include <stdint.h>

// design requirements
// Single producer (CAN_IRQs w same priority)
// Multiple Consumer (SD, ETH)
// the SD head must never miss a message (or else mark overflow)
// the ETH head does "best effort streaming"
// high throughput, lock free

/*
Assumptions
1. There is a single producer
2. The 

*/

// ! assumes the busses are SID
typedef struct { // 16 bytes
    uint32_t ticks_ms; // ms timestamp of reception
    uint16_t msg_id;   // message id 
    uint8_t bus_id;    // bus the message was rx'd on
    uint8_t dlc;       // data length code
    uint8_t data[8];   // message data
} timestamped_frame_t;

// todo determine appropriate values
static constexpr size_t SPMC_NUM_FRAMES = 256;
static constexpr size_t MIN_WRITE_FRAMES = 32;

typedef struct {
    timestamped_frame_t buffer[SPMC_NUM_FRAMES];
    volatile uint32_t head;

    // SD tail
    volatile uint32_t master_tail;
    TaskHandle_t master_task;

    // best-effort ETH tail
    volatile uint32_t follower_tail; 
    TaskHandle_t follower_task;

    // debug data
    uint32_t overflows;
} SPMC_t;
SPMC_t queue;

void SPMC_init(TaskHandle_t master_task, TaskHandle_t follower_task) {
    memset(&queue.buffer, 0, SPMC_NUM_FRAMES);


    queue.head = 0;

}

// ! note: the two ISRs must have the same priority to prevent interruption
int SPMC_push_back_ISR(timestamped_frame_t *incomming_frame) {
    uint32_t next_head = (queue.head + 1) % SPMC_NUM_FRAMES;

    if (next_head == queue.master_tail) {
        queue.overflows++;
        return -1;
    }

    // place the frame into the buffer

    if (queue.length > MIN_WRITE_FRAMES) {
        // Wake the SD card thread
    }

    return 0;
}


int SPMC_get_(size_t *size) {

}


int SPMC_pop_master() {
    // enter critical section
    
    
    // exit critical section
}

int SPMC_pop_follower() {
    // 

}

