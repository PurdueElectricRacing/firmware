#ifndef _FAULTS_H_
#define _FAULTS_H_

#include <stdint.h>

/* BEGIN AUTO FAULT STRUCTURES */
typedef struct
{
    struct {
        uint8_t dt_flow: 1;
        uint8_t bat_flow: 1;
    } warning;
    struct {
        uint8_t tv_timeout: 1;
        uint8_t pedal_dash_timeout: 1;
    } critical;
} faults_main_module_t;

typedef struct
{
    struct {
        uint8_t test_fault: 1;
    } critical;
    struct {
        uint8_t test_fault2: 1;
    } warning;
} faults_test_node_t;

/* END AUTO FAULT STRUCTURES */

#endif