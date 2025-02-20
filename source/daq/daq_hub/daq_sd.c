
#include "common/phal_F4_F7/rtc/rtc.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "daq_sd.h"
#include "daq_hub.h"
#include "ff.h"
#include "main.h"

static FRESULT sd_create_new_file(void);
static inline void sd_file_sync(void);
static void _sd_write_periodic(bool bypass);
static void sd_handle_error(sd_error_t err, FRESULT res);
static void sd_reset_error(void);
#define sd_write_periodic() _sd_write_periodic(false)

bool daq_request_sd_mount(void)
{
    return dh.sd_state == SD_STATE_MOUNTED || dh.sd_state == SD_STATE_ACTIVE;
}

static void sd_handle_error(sd_error_t err, FRESULT res)
{
    ++dh.sd_error_ct;
    dh.sd_last_err = err;
    dh.sd_last_err_res = res;
    dh.sd_last_error_time = getTick();
    PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 1);
    debug_printf("sd err: %d res: %d\n", err, res);
}

static void sd_reset_error(void)
{
    // Do not retry immediately
    if (!(getTick() - dh.sd_last_error_time > SD_ERROR_RETRY_MS)) return;

    dh.sd_last_error_time = getTick();
    if (dh.sd_last_err != SD_ERROR_NONE)
    {
        dh.sd_state = SD_STATE_IDLE; // Retry
        dh.sd_last_err = SD_ERROR_NONE;
        dh.sd_last_err_res = 0;
        PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 0);
    }
}

static FRESULT sd_create_new_file(void)
{
    FRESULT res;
    RTC_timestamp_t time;
    static uint32_t log_num;

    char f_name[30];
    if (PHAL_getTimeRTC(&time))
    {
        // Create file name from RTC
        sprintf(f_name, "log-20%02d-%02d-%02d--%02d-%02d-%02d.log",
                time.date.year_bcd, time.date.month_bcd,
                time.date.day_bcd,  time.time.hours_bcd,
                time.time.minutes_bcd, time.time.seconds_bcd);
    }
    else
    {
        sprintf(f_name, "log-%04d.log", log_num);
    }

    res = f_open(&dh.log_fp, f_name, FA_OPEN_APPEND | FA_READ | FA_WRITE);
    if (res == FR_OK) {
        debug_printf("delta: %d: created file %02d: %s\n", getTick() - dh.last_file_ms, log_num, f_name);
        log_num++;
        dh.last_file_ms = getTick();
    }
    else
    {
        sd_handle_error(SD_ERROR_FOPEN, res);
    }

    return res;
}

static inline void sd_file_sync(void)
{
    FRESULT res = f_sync(&dh.log_fp);
    if (res != FR_OK) sd_handle_error(SD_ERROR_SYNC, res);
}

static void _sd_write_periodic(bool bypass)
{
    timestamped_frame_t *buf;
    uint32_t consecutive_items;
    UINT bytes_written;
    FRESULT res;

    if (dh.sd_state == SD_STATE_ACTIVE)
    {
        // Use the total item count, not contiguous for the threshold
        if (bypass || bGetItemCount(&b_rx_can, RX_TAIL_SD) >= SD_MAX_WRITE_COUNT)
        {
            if ((bGetTailForRead(&b_rx_can, RX_TAIL_SD, (void**) &buf, &consecutive_items) == 0))
            {
                if (consecutive_items > SD_MAX_WRITE_COUNT) consecutive_items = SD_MAX_WRITE_COUNT; // limit
                // Write time :D
                PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 1);
                res = f_write(&dh.log_fp, buf, consecutive_items * sizeof(*buf), &bytes_written);
                if (res != FR_OK)
                {
                    sd_handle_error(SD_ERROR_WRITE, res);
                }
                else
                {
                    dh.last_write_ms = getTick();
                    bCommitRead(&b_rx_can, RX_TAIL_SD, bytes_written / sizeof(*buf));
                    sd_file_sync(); // fsync takes only 4 ticks and ensures sure cache is flushed on close
                }
                PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
            }
            else
            {
                daq_catch_error();
            }
        }
    }
}

void sd_shutdown(void)
{
    switch (dh.sd_state)
    {
        case SD_STATE_ACTIVE:
            _sd_write_periodic(true); // Finish write (bypass count limit)
            sd_file_sync();           // Flush cache
            f_close(&dh.log_fp);      // Close file
        case SD_STATE_MOUNTED:
            f_mount(0, "", 1);        // Unmount drive
            bDeactivateTail(&b_rx_can, RX_TAIL_SD);
        case SD_STATE_IDLE:
            SD_DeInit();              // Shutdown SDIO peripheral
        default:
            dh.sd_state = SD_STATE_IDLE;
            PHAL_writeGPIO(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, 0);
            PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
            PHAL_writeGPIO(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, 0);
            break;
    }
}

void sd_update_periodic(void)
{
    FRESULT res;

    dh.log_enable_sw = PHAL_readGPIO(LOG_ENABLE_PORT, LOG_ENABLE_PIN);
    if (!dh.log_enable_sw)
    {
        PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
        sd_shutdown();
        return;
    }
    else
    {
        PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
    }

    switch (dh.sd_state)
    {
        case SD_STATE_IDLE:
            if (SD_Detect() == SD_PRESENT)
            {
                res = f_mount(&dh.fat_fs, "", 1);
                if (res == FR_OK)
                {
                    bActivateTail(&b_rx_can, RX_TAIL_SD);
                    dh.sd_state = SD_STATE_MOUNTED;
                    PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);
                    debug_printf("SD UP!\n");
                }
                else
                {
                    sd_handle_error(SD_ERROR_MOUNT, res);
                    PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
                }
            }
            else
            {
                sd_handle_error(SD_ERROR_MOUNT, res);
                PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 0);
            }
            break;
        case SD_STATE_MOUNTED:
            res = sd_create_new_file();
            if (res == FR_OK)
            {
                dh.sd_state = SD_STATE_ACTIVE;
                dh.log_start_ms = getTick();
            }
            break;
        case SD_STATE_ACTIVE:
            if (getTick() - dh.last_write_ms > SD_WRITE_PERIOD_MS)
                sd_write_periodic();
            if (getTick() - dh.last_file_ms  > SD_NEW_FILE_PERIOD_MS)
                sd_create_new_file();
            break;
    }

    sd_reset_error();
}
