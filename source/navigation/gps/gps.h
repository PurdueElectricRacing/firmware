// /**
//  * @file gps.h
//  * @author Chris McGalliard (cmcgalli@purdue.edu)
//  * @brief
//  * @version 0.1
//  * @date 2022-12-28
//  *
//  *
//  */

#include <stdint.h>
#include "bsxlite_interface.h"
#include "can_parse.h"
#include "SFS.h"
#include "common/common_defs/common_defs.h"

#ifndef _GPS_H
#define _GPS_H

union i_Long
{
    uint8_t bytes[4];
    signed long iLong;
};

union i_Short
{
    uint8_t bytes[2];
    signed short iShort;
};

typedef struct
{
    vector_3d_t acceleration;
    vector_3d_t gyroscope;
    signed long messages_received;
    uint8_t raw_message[100];

    uint8_t g_speed_bytes[4];
    signed long g_speed;
    int16_t speed_rounded;

    uint8_t longitude_bytes[4];
    signed long longitude;
    int32_t lon_rounded;

    uint8_t latitude_bytes[4];
    signed long latitude;
    int32_t lat_rounded;

    uint8_t height_bytes[4];
    signed long height;
    int16_t height_rounded;

    uint8_t n_vel_bytes[4];
    signed long n_vel;
    int16_t n_vel_rounded;
    double n_vel_sfs1;
    double n_vel_sfs2;
    double n_hi;

    uint8_t e_vel_bytes[4];
    signed long e_vel;
    int16_t e_vel_rounded;

    uint8_t d_vel_bytes[4];
    signed long d_vel;
    uint16_t d_vel_rounded;

    uint8_t mag_dec_bytes[2];
    signed short mag_dec;
    uint8_t fix_type;
} GPS_Handle_t; // GPS handle

bool parseVelocity(GPS_Handle_t *GPS, ExtU *rtU);

#endif //_GPS_H