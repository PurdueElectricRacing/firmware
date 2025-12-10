#include "VCAN.h"

#define BUS2 (2)

// in the RX node:
typedef union {
    imu_gyro_data_t imu_gyro;
    // others here
} abox_can_data_t;

abox_can_data_t can_data;

[[gnu::always_inline]]
static inline void CAN_RECEIVE_imu_gyro(const uint64_t data_BE) {
    uint64_t data_LE = __built__builtin_bswap64(data_BE);

    // write(f"can_data.{message_name}.{signal_name} = ({type})((data_LE >> {start_bit}) & {mask})");
    can_data.imu_gyro.flag1 = (bool)((data_LE >> 0) & 0x1);
    can_data.imu_gyro.flag2 = (bool)((data_LE >> 1) & 0x1);
    can_data.imu_gyro.flag3 = (bool)((data_LE >> 2) & 0x1);
    can_data.imu_gyro.imu_version = (uint16_t)((data_LE >> 3) & 0xFFF);
    can_data.imu_gyro.imu_gyro_x = (uint16_t)((data_LE >> 15) & 0xFFF);
    can_data.imu_gyro.imu_gyro_y = (uint16_t)((data_LE >> 27) & 0xFFF);
    can_data.imu_gyro.imu_gyro_z = (uint16_t)((data_LE >> 39) & 0xFFF);
}

// theory crafting a new can send function
[[gnu::always_inline]]
static inline void CAN_SEND_imu_gyro(
    bool flag1, 
    bool flag2,
    bool flag3,
    uint16_t imu_version, // 12 bits
    uint16_t imu_gyro_x,  // 12 bits
    uint16_t imu_gyro_y,  // 12 bits
    uint16_t imu_gyro_z   // 12 bits
) {
    can_msg_t outgoing = {
        .bus = BUS2,
        .is_extended_id = true,
        .unmasked_id = IMU_GYRO_MSG_ID,
        .data_BE = 0
    };

    uint64_t data_LE = 0;

    // write(f"data_LE |= ((uint64_t)({signal_name} & {mask}) << {start_bit}");
    data_LE |= ((uint64_t)(flag1 & 0x1) << 0);
    data_LE |= ((uint64_t)(flag2 & 0x1) << 1);
    data_LE |= ((uint64_t)(flag3 & 0x1) << 2);
    data_LE |= ((uint64_t)(imu_version & 0xFFF) << 3);
    data_LE |= ((uint64_t)(imu_gyro_x & 0xFFF) << 15);
    data_LE |= ((uint64_t)(imu_gyro_y & 0xFFF) << 27);
    data_LE |= ((uint64_t)(imu_gyro_z & 0xFFF) << 39);

    outgoing.data_BE = __builtin_bswap64(data_LE);

    canTxSendToBack(&outgoing);
}

// old generator output
#define SEND_IMU_GYRO(imu_gyro_x_, imu_gyro_y_, imu_gyro_z_) do {
    CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_IMU_GYRO, .DLC=DLC_IMU_GYRO, .IDE=1};
    CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;
    data_a->imu_gyro.imu_gyro_x = imu_gyro_x_;
    data_a->imu_gyro.imu_gyro_y = imu_gyro_y_;
    data_a->imu_gyro.imu_gyro_z = imu_gyro_z_;
    canTxSendToBack(&msg);
} while(0)