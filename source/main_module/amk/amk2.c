#include "common/can_library/generated/MAIN_MODULE.h"

typedef enum : uint8_t {
    AMK_STATE_OFF     = 0,
    AMK_STATE_INIT    = 1,
    AMK_STATE_RUNNING = 2
} AMK_motor_state_t;

typedef struct {
    uint8_t AMK_bReserve1; /* Reserved */
    bool AMK_bInverterOn;  /* Controller Enable */
    bool AMK_bDcOn;        /* HV activiation */
    bool AMK_bEnable;      /* Driver Enable */
    bool AMK_bErrorReset;  /* Remove Error */
    bool AMK_bReserve2;    /* Reserved */
} AMK_Control_t;

typedef struct {
    void (*set_function)(void);
    void (*log_function)(void);
    INVA_SET_data_t *data; // sus, but all them should be the same
    AMK_Control_t control;
    AMK_motor_state_t state;
    // todo precharge
} amk_t;

void AMK_init(amk_t* amk, void (*set_function)(void), void (*log_function)(void), INVA_SET_data_t* data) {
    amk->state = AMK_STATE_OFF;
    amk->set_function = set_function;
    amk->log_function = log_function;
    amk->data = data;
    amk->control = (AMK_Control_t){0};
}

void AMK_reset(amk_t* amk) {
    amk->control.AMK_bErrorReset = true;
    amk->control.AMK_bInverterOn = false;
    
}

void AMK_periodic(amk_t* amk) {
    switch(amk->state) {
        case AMK_STATE_OFF:
        // transition logic ONLY, otherwise call the function
        break;
        case AMK_STATE_INIT:
        break;
        case AMK_STATE_RUNNING:
        break;
    }
}