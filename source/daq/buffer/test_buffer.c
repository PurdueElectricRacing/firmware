
#include "buffer.h"
#include "test_helper.h"

// Taken from daq_hub.h
typedef uint32_t canid_t;
typedef uint8_t busid_t;

typedef struct __attribute__((packed)) {
    uint32_t tick_ms; //!< ms timestamp of reception
    canid_t msg_id; //!< message id
    busid_t bus_id; //!< bus the message was rx'd on
    uint8_t dlc; //!< data length code
    uint8_t data[8]; //!< message data
} timestamped_frame_t;

#define RX_CT      32
#define RX_TAIL_CT 2
timestamped_frame_t rx_buffer[RX_CT];
b_tail_t tails[RX_TAIL_CT];

b_handle_t b_rx_can = {
    .buffer    = (uint8_t*)rx_buffer,
    .tails     = tails,
    .num_tails = RX_TAIL_CT,
};

static int _check_write(void) {
    t_start();
    timestamped_frame_t* head;
    uint32_t start_head = b_rx_can._head;
    uint32_t next_head  = (b_rx_can._head + 1) % RX_CT;
    uint32_t cont;
    t_check(bGetHeadForWrite(&b_rx_can, (void*)&head, &cont) == 0, "head");
    t_check(cont >= 1, "head cont");
    t_check(head == &rx_buffer[start_head], "rx buffer loc");

    // Writing
    t_check(bCommitWrite(&b_rx_can, 1) == 0, "commit write");
    t_check(b_rx_can._head == next_head, "head increments");

    t_end();
}

static int _test_basic() {
    t_start();

    // Initialize buffer
    bConstruct(&b_rx_can, sizeof(*rx_buffer), sizeof(rx_buffer));

    // Check initial conditions
    t_check(b_rx_can._item_size == 18, "Item size");
    t_check(b_rx_can._max_items == RX_CT, "Item count");
    t_check(b_rx_can._head == 0, "Head init");
    t_check(b_rx_can._min_tail == 0, "Min tail init");
    for (uint32_t i = 0; i < RX_TAIL_CT; ++i) {
        t_check(tails[i]._tail == 0, "Tail %lu points to 0", i);
        t_check(tails[i].active == false, "Tail %lu inactive", i);
    }

    bActivateTail(&b_rx_can, 0); // activate one tail
    t_check(tails[0].active == true, "tail activated");

    for (uint32_t i = 0; i < RX_CT - 1; ++i) {
        t_check(_check_write() == 0, "write 1 %d", i);
    }
    // Last one, should throw error
    timestamped_frame_t* head;
    uint32_t cont;
    t_check(bGetHeadForWrite(&b_rx_can, (void*)&head, &cont) != 0, "get head fails on full");
    t_check(cont == 0, "cont on full = 0");
    printf("cont: %d, head: %d, tail: %d\n", cont, b_rx_can._head, b_rx_can._min_tail);
    t_check(bCommitWrite(&b_rx_can, 1) != 0, "commit write fails on full");
    // Ensure the head didn't proceed forward
    t_check(b_rx_can._head == RX_CT - 1, "Head on full buffer");

    uint32_t contig_items;
    timestamped_frame_t* tail;
    t_check(bGetTailForRead(&b_rx_can, 0, (void*)&tail, &contig_items) == 0, "get tail");
    t_check(contig_items == RX_CT - 1, "contig");
    t_check(tail == rx_buffer, "init tail");
    t_check(bCommitRead(&b_rx_can, 0, contig_items) == 0, "commit");
    t_check(b_rx_can._min_tail == RX_CT - 1, "min tail update")

        t_check(_check_write() == 0, "write 1");

    t_check(bGetTailForRead(&b_rx_can, 0, (void*)&tail, &contig_items) == 0, "get tail");
    t_check(contig_items == 1, "contig");
    t_check(tail == &rx_buffer[RX_CT - 1], "init tail");
    t_check(bCommitRead(&b_rx_can, 0, 1) == 0, "commit");
    t_check(b_rx_can._min_tail == 0, "min tail update");

    // center the tail
    b_rx_can._min_tail = 16;
    tails[0]._tail     = 16;

    // Tail is now greater than head
    t_check(bGetTailForRead(&b_rx_can, 0, (void*)&tail, &contig_items) == 0, "get tail");
    t_check(contig_items == (RX_CT - 16), "contig");
    t_check(tail == &rx_buffer[16], "init tail");
    t_check(bCommitRead(&b_rx_can, 0, 16) == 0, "commit");
    t_check(b_rx_can._min_tail == 0, "min tail update");

    // TODO: test successful wrap around
    // TODO: test trying to read past contiguious

    t_end();
}

int main(int argc, char* argv[]) {
    t_run(_test_basic);

    return 0;
}
