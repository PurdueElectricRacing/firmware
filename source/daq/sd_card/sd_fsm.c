#include "common/phal/gpio.h"

typedef enum {
    SD_STATE_DISABLED      = 0,
    SD_STATE_WAIT_FOR_CARD = 1,
    SD_STATE_MOUNTING      = 2,
    SD_STATE_OPENING_FILE  = 3,
    SD_STATE_READY2LOG     = 4,
    SD_STATE_WRITING       = 5,
    SD_STATE_CLOSING_FILE  = 6,
    SD_STATE_UNMOUNTING    = 7,
    SD_STATE_RECOVERING    = 8,
    SD_STATE_FATAL         = 9
} sd_state_t;

sd_state_t sd_state = SD_STATE_DISABLED;
sd_state_t next_sd_state = SD_STATE_DISABLED;

void sd_fsm_periodic(void) {
    // set default states
    sd_state = next_sd_state;
    next_sd_state = sd_state;

    bool is_logging_enabled = PHAL_readGPIO(LOG_ENABLE_PORT, LOG_ENABLE_PIN);

    switch (sd_state) {
        case SD_STATE_DISABLED: {

            if (is_logging_enabled) {
                next_sd_state = SD_STATE_WAIT_FOR_CARD;
            }
            break;
        }
        case SD_STATE_WAIT_FOR_CARD: {

            bool is_card_detected = false; // todo check sd detect pin
            if (is_card_detected) {
                next_sd_state = SD_STATE_MOUNTING;
            }
            break;
        }
        case SD_STATE_MOUNTING: {

            bool is_mounted = false; // todo call f_mount()
            if (is_mounted) {
                next_sd_state = SD_STATE_OPENING_FILE;
            }
            break;
        }
        case SD_STATE_OPENING_FILE: {
            bool is_open_successful = false; // todo call f_open()
            if (is_open_successful) {
                next_sd_state = SD_STATE_READY2LOG;
            }
            break;
        }
        case SD_STATE_READY2LOG: {
            // block on notification from SPMC

            // peek chunks
            // begin f_write() of the chunk with DMA
            if (!is_logging_enabled) {
                next_sd_state = SD_STATE_CLOSING_FILE;
            } else {
                next_sd_state = SD_STATE_WRITING;
            }
            break;
        }
        case SD_STATE_WRITING: {
            // block on the DMA transfer to finish via notify

            bool file_is_large_enough = false;
            if (file_is_large_enough || !is_logging_enabled) {
                next_sd_state = SD_STATE_CLOSING_FILE;
            } else {
                next_sd_state = SD_STATE_READY2LOG;
            }
            break;
        }
        case SD_STATE_CLOSING_FILE: {
            // call f_close()
            // call f_sync()

            if (!is_logging_enabled) {
                next_sd_state = SD_STATE_UNMOUNTING;
            } else {
                next_sd_state = SD_STATE_OPENING_FILE;
            }
            break;
        }
        case SD_STATE_UNMOUNTING: {
            // call unmount

            next_sd_state = SD_STATE_DISABLED;
            break;
        }
        case SD_STATE_RECOVERING: {
            break;
        }
        case SD_STATE_FATAL: {
            break;
        }
    }
}