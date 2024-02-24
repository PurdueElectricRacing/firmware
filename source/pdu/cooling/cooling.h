#ifndef _COOLING_H_
#define _COOLING_H_
#include "can_parse.h"
#include "common/phal_F4_F7/can/can.h"

typedef struct {
    uint16_t fan1_speed;
    uint16_t fan2_speed;
    bool pump1_status;
    bool pump2_status;
    bool fan1_status;
    bool fan2_status;
    bool aux_status;
} cooling_request_t;
void update_cooling_periodic();
void cooling_driver_request_CALLBACK(CanParsedData_t *msg_data_a);
#endif
