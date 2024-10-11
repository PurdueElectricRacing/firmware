/**
 * @file amk.h
 * @author Cole Roberts (rober638@purdue.edu)
 * @brief  Vroom
 * @version 0.1
 * @date 2024-10-11
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _AMK_H_
#define _AMK_H_

#include <stdbool.h>
#include <stdint.h>

/* Inverter -> CAN */
typedef struct
{
    uint16_t AMK_bReserve        : 8;
    uint16_t AMK_bSystemReady    : 1;
    uint16_t AMK_bError          : 1;
    uint16_t AMK_bWarn           : 1;
    uint16_t AMK_bQuitDcOn       : 1;
    uint16_t AMK_bDcOn           : 1; /* Same as QUE ?? */
    uint16_t AMK_bQuitInverterOn : 1;
    uint16_t AMK_bInverterOn     : 1;
    uint16_t AMK_bDerating       : 1;
} AMK_Status_t;

/* CAN -> Inverter */
typedef struct
{
    uint16_t AMK_bReserve1   : 8;
    uint16_t AMK_bInverterOn : 1;
    uint16_t AMK_bDcOn       : 1;
    uint16_t AMK_bEnable     : 1;
    uint16_t AMK_bErrorReset : 1;
    uint16_t AMK_bReserve2   : 1;
} AMK_Control_t;

#endif /* _AMK_H_ */
