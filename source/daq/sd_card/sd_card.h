/**
 * @file sd_card.h
 * @brief Logging of received bus messages onto an SD card
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#ifndef SD_CARD_H
#define SD_CARD_H

#include "external/fatfs/ff.h"
#include "common/freertos/freertos.h"

typedef enum {
    SD_STATE_IDLE    = 0,
    SD_STATE_MOUNTED = 1,
    SD_STATE_ACTIVE  = 2,
} sd_state_t;

typedef enum {
    SD_ERROR_NONE   = 0,
    SD_ERROR_MOUNT  = 1,
    SD_ERROR_FOPEN  = 2,
    SD_ERROR_FCLOSE = 3,
    SD_ERROR_WRITE  = 4,
    SD_ERROR_DETEC  = 5,
    SD_ERROR_SYNC   = 6,
} sd_error_t;

typedef struct {
    // SD Card
    sd_state_t sd_state;
    FATFS fat_fs;
    uint32_t sd_error_ct;
    sd_error_t sd_last_err;
    FRESULT sd_last_err_res;
    uint32_t sd_last_error_time;
    osThreadId_t sd_task_handle;

    FIL log_fp;
    uint32_t log_start_ms;
    uint32_t last_write_ms;
    uint32_t last_file_ms;
    bool log_enable_sw; //!< Debounced switch state
    bool log_enable_tcp;
} SD_manager_t;
extern SD_manager_t sd_manager;

bool daq_request_sd_mount(void);
void sd_shutdown(void);
void sd_update_periodic(void);

#endif // SD_CARD_H
