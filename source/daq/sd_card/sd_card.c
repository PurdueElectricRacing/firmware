/**
 * @file sd.c
 * @brief Logging of received bus messages onto an SD card
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#include "sd_card.h"

#include "common/phal/gpio.h"
#include "common/phal/rtc.h"
#include "ff.h"
#include "main.h"
#include "common/sdio/sdio.h"
#include <stdio.h>
#include "spmc.h"

static constexpr uint32_t SD_WRITE_PERIOD_MS = 100;
static constexpr uint32_t SD_NEW_FILE_PERIOD_MS = 1 * 60 * 1000; // 1 min
static constexpr uint32_t SD_ERROR_RETRY_MS = 250;

SD_manager_t sd_manager = {
    // SD Card
    .sd_state           = SD_STATE_IDLE,
    .sd_error_ct        = 0,
    .sd_last_error_time = 0,
    .sd_last_err        = SD_ERROR_NONE,
    .sd_last_err_res    = 0,
    .sd_task_handle     = NULL,
    .last_file_ms       = 0,
    .last_write_ms      = 0,
    .log_enable_sw      = false
};

bool daq_request_sd_mount(void) {
    return sd_manager.sd_state == SD_STATE_MOUNTED || sd_manager.sd_state == SD_STATE_ACTIVE;
}

static void sd_handle_error(sd_error_t sd_error, FRESULT result) {
    ++sd_manager.sd_error_ct;
    sd_manager.sd_last_err        = sd_error;
    sd_manager.sd_last_err_res    = result;
    sd_manager.sd_last_error_time = xTaskGetTickCount();
    PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 1);
}

static void sd_reset_error(void) {
    // Do not retry immediately
    if (!(xTaskGetTickCount() - sd_manager.sd_last_error_time > SD_ERROR_RETRY_MS)) {
        return;
    }

    sd_manager.sd_last_error_time = xTaskGetTickCount();
    if (sd_manager.sd_last_err != SD_ERROR_NONE) {
        sd_manager.sd_state        = SD_STATE_IDLE; // Retry
        sd_manager.sd_last_err     = SD_ERROR_NONE;
        sd_manager.sd_last_err_res = 0;
        PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 0);
    }
}

static FRESULT sd_create_new_file(void) {
    FRESULT result;
    RTC_timestamp_t time;
    static uint32_t log_num;

    char f_name[30];
    if (PHAL_getTimeRTC(&time)) {
        // Create file name from RTC
        sprintf(
            f_name, 
            "log-20%02d-%02d-%02d--%02d-%02d-%02d.log", 
            time.date.year_bcd,
            time.date.month_bcd,
            time.date.day_bcd,
            time.time.hours_bcd,
            time.time.minutes_bcd,
            time.time.seconds_bcd
        );
    } else {
        sprintf(f_name, "log-%0ld.log", (unsigned long)log_num);
    }

    result = f_open(&sd_manager.log_fp, f_name, FA_OPEN_APPEND | FA_READ | FA_WRITE);
    if (result != FR_OK) {
        sd_handle_error(SD_ERROR_FOPEN, result);
        return result;
    }

    log_num++;
    sd_manager.last_file_ms = xTaskGetTickCount();

    return result;
}

static inline void sd_file_sync(void) {
    FRESULT res = f_sync(&sd_manager.log_fp);
    if (res != FR_OK) {
        sd_handle_error(SD_ERROR_SYNC, res);
    }
}

// todo reevaluate the logic here
static void sd_write_periodic() {
    if (sd_manager.sd_state != SD_STATE_ACTIVE) {
        return;
    }

    // Use the unread item count, not contiguous for the threshold
    timestamped_frame_t *frame; // updated by SPMC_master_peek_chunk() on success

    if (!SPMC_master_peek_chunk(&g_spmc, &frame)) {
        return;
    }

    // Write time :D
    PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 1);

    UINT bytes_written; // updated to by f_write()
    size_t total_bytes = SPMC_CHUNK_NUM_FRAMES * sizeof(timestamped_frame_t);
    FRESULT result     = f_write(&sd_manager.log_fp, frame, total_bytes, &bytes_written);
    if (result != FR_OK) {
        // todo check bytes written
        sd_handle_error(SD_ERROR_WRITE, result);
    } else {
        // success
        sd_manager.last_write_ms = xTaskGetTickCount();
        SPMC_master_advance_tail(&g_spmc);
        sd_file_sync(); // fsync takes only 4 ticks and ensures sure cache is flushed on close
    }

    PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
}

void sd_shutdown(void) {
    switch (sd_manager.sd_state) {
        case SD_STATE_ACTIVE:
            // sd_write_periodic(true); // Finish write (bypass count limit)
            sd_file_sync(); // Flush cache
            f_close(&sd_manager.log_fp); // Close file
            // ! intentional fall through
        case SD_STATE_MOUNTED:
            f_mount(0, "", 1); // Unmount drive
            // ! intentional fall through
        case SD_STATE_IDLE:
            SD_DeInit(); // Shutdown SDIO peripheral
            // ! intentional fall through
        default:
            sd_manager.sd_state = SD_STATE_IDLE;
            PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
            PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
            // PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 0);
            break;
    }
}

void sd_update_periodic(void) {
    FRESULT result;

    sd_manager.log_enable_sw = PHAL_readGPIO(LOG_ENABLE_PORT, LOG_ENABLE_PIN);
    if (!sd_manager.log_enable_sw) {
        PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
        sd_shutdown();
        return;
    } else {
        PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
    }

    switch (sd_manager.sd_state) {
        case SD_STATE_IDLE:
            if (SD_Detect() != SD_PRESENT) {
                PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
                break;
            }

            result = f_mount(&sd_manager.fat_fs, "", 1);
            if (result != FR_OK) {
                sd_handle_error(SD_ERROR_MOUNT, result);
                PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
                break;
            }

            sd_manager.sd_state = SD_STATE_MOUNTED;
            PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
            break;
        case SD_STATE_MOUNTED:
            result = sd_create_new_file();
            if (result == FR_OK) {
                sd_manager.sd_state     = SD_STATE_ACTIVE;
                sd_manager.log_start_ms = xTaskGetTickCount();
            }
            break;
        case SD_STATE_ACTIVE:
            if (xTaskGetTickCount() - sd_manager.last_write_ms > SD_WRITE_PERIOD_MS) {
                sd_write_periodic();
            }

            if (xTaskGetTickCount() - sd_manager.last_file_ms > SD_NEW_FILE_PERIOD_MS) {
                sd_create_new_file();
            }
            break;
    }

    sd_reset_error();
}
