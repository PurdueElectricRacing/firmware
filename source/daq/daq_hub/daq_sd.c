
#include "daq_sd.h"

#include "common/phal/gpio.h"
#include "common/phal/rtc.h"
#include "daq_hub.h"
#include "ff.h"
#include "main.h"

static FRESULT sd_create_new_file(void);
static inline void sd_file_sync(void);
static void _sd_write_periodic(bool bypass);
static void sd_handle_error(sd_error_t sd_error, FRESULT result);
static void sd_reset_error(void);
#define sd_write_periodic() _sd_write_periodic(false)

bool daq_request_sd_mount(void) {
    return daq_hub.sd_state == SD_STATE_MOUNTED || daq_hub.sd_state == SD_STATE_ACTIVE;
}

static void sd_handle_error(sd_error_t sd_error, FRESULT result) {
    ++daq_hub.sd_error_ct;
    daq_hub.sd_last_err        = sd_error;
    daq_hub.sd_last_err_res    = result;
    daq_hub.sd_last_error_time = getTick();
    PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 1);
    debug_printf("sd err: %d res: %d\n", sd_error, result);
}

static void sd_reset_error(void) {
    // Do not retry immediately
    if (!(getTick() - daq_hub.sd_last_error_time > SD_ERROR_RETRY_MS)) {
        return;
    }

    daq_hub.sd_last_error_time = getTick();
    if (daq_hub.sd_last_err != SD_ERROR_NONE) {
        daq_hub.sd_state        = SD_STATE_IDLE; // Retry
        daq_hub.sd_last_err     = SD_ERROR_NONE;
        daq_hub.sd_last_err_res = 0;
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
        sprintf(f_name, "log-20%02d-%02d-%02d--%02d-%02d-%02d.log", time.date.year_bcd, time.date.month_bcd, time.date.day_bcd, time.time.hours_bcd, time.time.minutes_bcd, time.time.seconds_bcd);
    } else {
        sprintf(f_name, "log-%04d.log", log_num);
    }

    result = f_open(&daq_hub.log_fp, f_name, FA_OPEN_APPEND | FA_READ | FA_WRITE);
    if (result != FR_OK) {
        sd_handle_error(SD_ERROR_FOPEN, result);
        return result;
    }

    debug_printf("delta: %d: created file %02d: %s\n", getTick() - daq_hub.last_file_ms, log_num, f_name);
    log_num++;
    daq_hub.last_file_ms = getTick();

    return result;
}

static inline void sd_file_sync(void) {
    FRESULT res = f_sync(&daq_hub.log_fp);
    if (res != FR_OK) {
        sd_handle_error(SD_ERROR_SYNC, res);
    }
}

static void _sd_write_periodic(bool bypass) {
    timestamped_frame_t* buf;
    uint32_t consecutive_items;
    UINT bytes_written;
    FRESULT result;

    if (daq_hub.sd_state != SD_STATE_ACTIVE)
        return;

    // Use the total item count, not contiguous for the threshold
    if (!(bypass || bGetItemCount(&b_rx_can, RX_TAIL_SD) >= SD_MAX_WRITE_COUNT)) {
        return;
    }

    if (bGetTailForRead(&b_rx_can, RX_TAIL_SD, (void**)&buf, &consecutive_items) != 0) {
        daq_hub.sd_rx_overflow++;
        return;
    }

    if (consecutive_items > SD_MAX_WRITE_COUNT) {
        consecutive_items = SD_MAX_WRITE_COUNT; // enforce the limit
    }
        
    // Write time :D
    PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 1);
    result = f_write(&daq_hub.log_fp, buf, consecutive_items * sizeof(*buf), &bytes_written);
    if (result != FR_OK) {
        sd_handle_error(SD_ERROR_WRITE, result);
    } else {
        daq_hub.last_write_ms = getTick();
        bCommitRead(&b_rx_can, RX_TAIL_SD, bytes_written / sizeof(*buf));
        sd_file_sync(); // fsync takes only 4 ticks and ensures sure cache is flushed on close
    }
    PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
}

void sd_shutdown(void) {
    switch (daq_hub.sd_state) {
        case SD_STATE_ACTIVE:
            // _sd_write_periodic(true); // Finish write (bypass count limit)
            sd_file_sync(); // Flush cache
            f_close(&daq_hub.log_fp); // Close file
        case SD_STATE_MOUNTED:
            f_mount(0, "", 1); // Unmount drive
            bDeactivateTail(&b_rx_can, RX_TAIL_SD);
        case SD_STATE_IDLE:
            SD_DeInit(); // Shutdown SDIO peripheral
        default:
            daq_hub.sd_state = SD_STATE_IDLE;
            PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
            PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
            PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 0);
            break;
    }
}

void sd_update_periodic(void) {
    FRESULT result;

    daq_hub.log_enable_sw = PHAL_readGPIO(LOG_ENABLE_PORT, LOG_ENABLE_PIN);
    if (!daq_hub.log_enable_sw) {
        PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
        sd_shutdown();
        return;
    } else {
        PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
    }

    switch (daq_hub.sd_state) {
        case SD_STATE_IDLE:
            if (SD_Detect() != SD_PRESENT) {
                PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
                break;
            }

            result = f_mount(&daq_hub.fat_fs, "", 1);
            if (result != FR_OK) {
                sd_handle_error(SD_ERROR_MOUNT, result);
                PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
                break;
            }

            bActivateTail(&b_rx_can, RX_TAIL_SD);
            daq_hub.sd_state = SD_STATE_MOUNTED;
            PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
            debug_printf("SD UP!\n");
            break;
        case SD_STATE_MOUNTED:
            result = sd_create_new_file();
            if (result == FR_OK) {
                daq_hub.sd_state     = SD_STATE_ACTIVE;
                daq_hub.log_start_ms = getTick();
            }
            break;
        case SD_STATE_ACTIVE:
            if (getTick() - daq_hub.last_write_ms > SD_WRITE_PERIOD_MS) {
                sd_write_periodic();
            }

            if (getTick() - daq_hub.last_file_ms > SD_NEW_FILE_PERIOD_MS) {
                sd_create_new_file();
            }
            break;
    }

    sd_reset_error();
}
