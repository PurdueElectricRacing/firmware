#ifndef _FAULTS_H_
#define _FAULTS_H_

#include "common/phal_l4/can/can.h"

/* BEGIN AUTO FAULT DEFS */
typedef enum {
    FLT_C_TEST,
    FLT_W_TEST2
} faults_t;
/* END AUTO FAULT DEFS */

extern uint64_t faults;

#define SET_FAULT(_fault) (faults |= 1ull << _fault)
#define CLR_FAULT(_fault) (faults &= ~(1ull << _fault))
#define GET_FAULT(_fault) ((faults >> _fault) & 1ull)
#define UPD_FAULT(_fault, _val) if (_val) SET_FAULT(_fault); else CLR_FAULT(_fault)

#endif