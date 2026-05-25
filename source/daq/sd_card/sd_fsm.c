#include "common/phal/gpio.h"
#include "common/phal/rtc.h"
#include "common/sdio/sdio.h"
#include <stdio.h>
#include "external/fatfs/ff.h"

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
FATFS fat_fs;
FIL file_pointer;
int retry_attempts = 0;

static void get_next_filename(char *buffer, size_t buffer_size) {
    static int log_num = 0;
    RTC_timestamp_t time = {0};

    if (PHAL_getTimeRTC(&time)) {
        // Create file name from RTC
        sprintf(
            buffer, 
            "log-20%02d-%02d-%02d--%02d-%02d-%02d.log", 
            time.date.year_bcd,
            time.date.month_bcd,
            time.date.day_bcd,
            time.time.hours_bcd,
            time.time.minutes_bcd,
            time.time.seconds_bcd
        );
    } else {
        sprintf(buffer, "log-%d.log", log_num);
        log_num++;
    }
}

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

            if (SD_Detect() == SD_PRESENT) {
                next_sd_state = SD_STATE_MOUNTING;
            }
            break;
        }
        case SD_STATE_MOUNTING: {

            if (f_mount(&fat_fs, "", 1) == FR_OK) {
                next_sd_state = SD_STATE_OPENING_FILE;
            } else {
                next_sd_state = SD_STATE_RECOVERING;
            }
            break;
        }
        case SD_STATE_OPENING_FILE: {
            char filename[30];
            get_next_filename(filename, sizeof(filename));

            if (f_open(&file_pointer, filename, FA_OPEN_APPEND | FA_READ | FA_WRITE) == FR_OK) {
                next_sd_state = SD_STATE_READY2LOG;
            } else {
                next_sd_state = SD_STATE_RECOVERING;
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
            // recovery action?
            
            retry_attempts++;
            if (recovery_success) {
                retry_attempts = 0;
                next_sd_state = SD_STATE_DISABLED;
            } else if (retry_attempts > 3) {
                next_sd_state = SD_STATE_FATAL;
            }
            break;
        }
        case SD_STATE_FATAL: {
            PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 1);
            break;
        }
    }
}