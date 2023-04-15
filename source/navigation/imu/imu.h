/**
 * @file imu.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Integration of the bmi and bsxlite filter software provided by Bosch
 * @version 0.1
 * @date 2022-10-05
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _IMU_H_
#define _IMU_H_

#include "bmi088.h"
#include "bsxlite_interface.h"
#include "common/psched/psched.h"
#include "can_parse.h"
#include <stdbool.h>
#include <float.h>
#include "SFS.h"

typedef struct
{
    BMI088_Handle_t *bmi;
    bsxlite_instance_t inst;
    bsxlite_out_t output;
    bsxlite_version version;
    bsxlite_return_t last_result;
} IMU_Handle_t;

bool imu_init(IMU_Handle_t *imu_h);
void imu_periodic(IMU_Handle_t *imu_h, ExtU *rtU);

#endif