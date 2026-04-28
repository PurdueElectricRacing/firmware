/**
 * @file sd.h
 * @brief Logging of received bus messages onto an SD card
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Eileen Yoon (eyn@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#ifndef SD_CARD_H
#define SD_CARD_H

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

bool daq_request_sd_mount(void);
void sd_shutdown(void);
void sd_update_periodic(void);

#endif // SD_CARD_H
