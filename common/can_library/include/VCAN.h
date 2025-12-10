#include <stdint.h>
#include <stdbool.h>

#define VCAN_BAUD_RATE (500000)

#define IMU_GYRO_MSG_ID (0x03C)
typedef struct {
    bool flag1;
    bool flag2;
    bool flag3;
    uint16_t imu_version; // 12 bits
    uint16_t imu_gyro_x;  // 12 bits
    uint16_t imu_gyro_y;  // 12 bits
    uint16_t imu_gyro_z;  // 12 bits
} imu_gyro_data_t;

#define IMU_ACCEL_MSG_ID (0x03D)
// others...

typedef struct {
    uint8_t bus;
    bool is_extended_id;
    uint32_t unmasked_id;
    uint64_t data_BE;
} can_msg_t;