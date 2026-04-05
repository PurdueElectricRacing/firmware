#ifndef IZZE_IMU_H
#define IZZE_IMU_H

#include <stdint.h>

static constexpr uint16_t IZZE_IMU_PROGRAMMING_CONSTANT = 0x7530;

typedef enum : uint8_t {
    GYRO_SCALE_125DPS  = 1,
    GYRO_SCALE_250DPS  = 2,
    GYRO_SCALE_500DPS  = 3,
    GYRO_SCALE_1000DPS = 4,
    GYRO_SCALE_2000DPS = 5,
    GYRO_SCALE_4000DPS = 6
} gyro_scale_t;

typedef enum : uint8_t {
    ACCEL_SCALE_2G  = 1,
    ACCEL_SCALE_4G  = 2,
    ACCEL_SCALE_8G  = 3,
    ACCEL_SCALE_16G = 4
} accel_scale_t;

typedef enum : uint8_t {
    ODR_12HZ   = 1,
    ODR_26HZ   = 2,
    ODR_52HZ   = 3,
    ODR_104HZ  = 4,
    ODR_208HZ  = 5,
    ODR_416HZ  = 6,
    ODR_833HZ  = 7,
    ODR_1667HZ = 8,
    ODR_3333HZ = 9,
    ODR_6667HZ = 10,
} output_data_rate_t;

typedef enum : uint8_t {
    BIT_RATE_1MBPS   = 1,
    BIT_RATE_500KBPS = 2,
    BIT_RATE_250KBPS = 3,
    BIT_RATE_125KBPS = 4,
} bit_rate_t;

#endif // IZZE_IMU_H