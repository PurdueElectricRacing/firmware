/**
 * @file sd_fsm.c
 * @brief Writing of received bus messages onto an SD card
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdio.h>
#include "common/phal/gpio.h"
#include "common/phal/rtc.h"
#include "common/sdio/sdio.h"
#include "external/fatfs/ff.h"
#include "spmc.h"
#include "common/freertos/freertos.h"

typedef enum {
    SD_STATE_DISABLED     = 0,
    SD_STATE_INSERT_CARD  = 1,
    SD_STATE_MOUNTING     = 2,
    SD_STATE_OPENING_FILE = 3,
    SD_STATE_READY2LOG    = 4,
    SD_STATE_CLOSING_FILE = 5,
    SD_STATE_UNMOUNTING   = 6,
    SD_STATE_RECOVERING   = 7,
    SD_STATE_FATAL        = 8
} sd_state_t;

static constexpr uint32_t SD_NEW_FILE_PERIOD_MS = 1 * 60 * 1000; // 1 min

sd_state_t sd_state = SD_STATE_DISABLED;
sd_state_t next_sd_state = SD_STATE_DISABLED;
FATFS fat_fs;
FIL file_object;
bool is_file_open = false;
bool is_fs_mounted = false;
int recovery_attempts = 0;
uint32_t last_open_time_ms = 0;

static void get_next_filename(char *buffer, size_t buffer_size) {
    static int log_num = 0;
    RTC_timestamp_t time = {0};

    if (PHAL_getTimeRTC(&time)) {
        // Create file name from RTC
        snprintf(
            buffer, 
            buffer_size,
            "log-20%02d-%02d-%02d--%02d-%02d-%02d.log", 
            time.date.year_bcd,
            time.date.month_bcd,
            time.date.day_bcd,
            time.time.hours_bcd,
            time.time.minutes_bcd,
            time.time.seconds_bcd
        );
    } else {
        snprintf(buffer, buffer_size, "log-%d.log", log_num);
        log_num++;
    }
}

static void release_resources() {
    if (is_file_open) {
        f_sync(&file_object);
        f_close(&file_object);
        is_file_open = false;
    }

    if (is_fs_mounted) {
        f_mount(NULL, "", 1); // unmount
        is_fs_mounted = false;
    }
}

void sd_fsm_periodic(void) {
    // set default states
    sd_state = next_sd_state;
    next_sd_state = sd_state;

    bool is_logging_enabled = PHAL_readGPIO(LOG_ENABLE_PORT, LOG_ENABLE_PIN);
    // todo: power loss begins a close and unmount sequence

    switch (sd_state) {
        case SD_STATE_DISABLED: {

            if (is_logging_enabled) {
                next_sd_state = SD_STATE_INSERT_CARD;
            }
            break;
        }
        case SD_STATE_INSERT_CARD: {

            if (!is_logging_enabled) {
                next_sd_state = SD_STATE_DISABLED;
            } else if (SD_Detect() == SD_PRESENT) {
                next_sd_state = SD_STATE_MOUNTING;
            }
            break;
        }
        case SD_STATE_MOUNTING: {

            if (f_mount(&fat_fs, "", 1) == FR_OK) {
                is_fs_mounted = true;
                next_sd_state = SD_STATE_OPENING_FILE;
            } else {
                next_sd_state = SD_STATE_RECOVERING;
            }
            break;
        }
        case SD_STATE_OPENING_FILE: {
            char filename[30];
            get_next_filename(filename, sizeof(filename));

            if (f_open(&file_object, filename, FA_OPEN_APPEND | FA_READ | FA_WRITE) == FR_OK) {
                is_file_open = true;
                last_open_time_ms = xTaskGetTickCount();
                next_sd_state = SD_STATE_READY2LOG;
            } else {
                next_sd_state = SD_STATE_RECOVERING;
            }
            break;
        }
        case SD_STATE_READY2LOG: {
            // todo: block on notification from SPMC, polling for now
            timestamped_frame_t *first_frame; // updated by SPMC_master_peek_chunk()
            bool chunk_available = SPMC_master_peek_chunk(&g_spmc, &first_frame);
            if (chunk_available) {
                // f_write() calls with DMA then blocks, no need to wait for completion notification
                PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 1);
                UINT bytes_to_write = SPMC_BYTES_PER_CHUNK;
                UINT bytes_written  = 0; // updated to by f_write()
                FRESULT result      = f_write(&file_object, first_frame, bytes_to_write, &bytes_written);
                PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
                if (result != FR_OK) {
                    // todo check bytes written
                    next_sd_state = SD_STATE_RECOVERING;
                    break;
                }
                // success
                SPMC_master_advance_tail(&g_spmc);
                recovery_attempts = 0; // reset recovery attempts on success
            }

            bool is_new_file_period_elapsed = xTaskGetTickCount() - last_open_time_ms > SD_NEW_FILE_PERIOD_MS;
            if (!is_logging_enabled || is_new_file_period_elapsed) {
                next_sd_state = SD_STATE_CLOSING_FILE;
            }
            break;
        }
        case SD_STATE_CLOSING_FILE: {
            if (f_sync(&file_object) != FR_OK) {
                next_sd_state = SD_STATE_RECOVERING;
                break;
            }
            if (f_close(&file_object) != FR_OK) {
                next_sd_state = SD_STATE_RECOVERING;
                break;
            }

            is_file_open = false;

            if (!is_logging_enabled) {
                next_sd_state = SD_STATE_UNMOUNTING;
                is_fs_mounted = false;
            } else {
                next_sd_state = SD_STATE_OPENING_FILE;
            }
            break;
        }
        case SD_STATE_UNMOUNTING: {
            f_mount(NULL, "", 1); // unmount

            next_sd_state = SD_STATE_DISABLED;
            break;
        }
        case SD_STATE_RECOVERING: {
            release_resources();
            osDelay(200); // wait a bit before trying again
            recovery_attempts++;

            if (recovery_attempts > 3) {
                next_sd_state = SD_STATE_FATAL;
            } else {
                next_sd_state = SD_STATE_DISABLED;
            }
            break;
        }
        case SD_STATE_FATAL: {
            PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 1);
            break;
        }
    }
}