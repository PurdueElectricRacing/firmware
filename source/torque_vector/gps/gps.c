#include "gps.h"

void gps_thread(void) {
    // block until woken by dma interrupt

    // mark the current time

    // validate the gps header
        // return if invalid

    // identify the gps message type

    // dispatch to the correct decoder
}