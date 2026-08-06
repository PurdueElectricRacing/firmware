#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

#include "daqapp_bridge.hpp"
#include "firmware_can_bridge.h"

namespace fs = std::filesystem;

namespace {

fs::path find_vcan_dbc() {
    const fs::path directory = fs::path(PROJECT_ROOT) / "firmware/can_library/dbc";
    fs::path result;
    for (const auto &entry : fs::directory_iterator(directory)) {
        const std::string name = entry.path().filename().string();
        if (entry.is_regular_file() && name.rfind("VCAN_", 0) == 0 && entry.path().extension() == ".dbc") {
            EXPECT_TRUE(result.empty()) << "multiple generated VCAN DBC files found";
            result = entry.path();
        }
    }
    EXPECT_FALSE(result.empty()) << "generated VCAN DBC not found";
    return result;
}

}  // namespace

TEST(CanContract, FirmwareTransmitDecodesInDaqapp) {
    std::array<std::uint8_t, 16> timestamped_frame {};
    ASSERT_TRUE(per_fw_send_main_hb(5, 42, timestamped_frame.data()));

    PerCanFrame parsed {};
    ASSERT_TRUE(per_daq_parse_udp_frame(timestamped_frame.data(), timestamped_frame.size(), &parsed));
    EXPECT_EQ(parsed.id, per_fw_main_hb_id());
    EXPECT_EQ(parsed.is_extended, 0);
    EXPECT_EQ(parsed.length, 8);
    EXPECT_EQ(parsed.data[0], 5);

    const fs::path dbc = find_vcan_dbc();
    double car_state = 0.0;
    ASSERT_TRUE(per_daq_decode_main_hb(dbc.c_str(), &parsed, &car_state));
    EXPECT_DOUBLE_EQ(car_state, 5.0);
}

TEST(CanContract, DaqappTransmitDispatchesInFirmware) {
    const fs::path dbc = find_vcan_dbc();
    PerCanFrame encoded {};
    ASSERT_TRUE(per_daq_encode_start_button(dbc.c_str(), true, &encoded));
    EXPECT_EQ(encoded.id, per_fw_start_button_id());
    EXPECT_EQ(encoded.is_extended, 0);
    EXPECT_EQ(encoded.length, 1);
    EXPECT_EQ(encoded.data[0], 1);

    ASSERT_TRUE(per_fw_dispatch_can(encoded.id, encoded.data, encoded.length, 1234));
    EXPECT_TRUE(per_fw_start_button_pressed());
    EXPECT_EQ(per_fw_start_button_last_rx(), 1234U);
}

TEST(CanContract, FirmwareDispatcherIgnoresUnknownMessage) {
    std::array<std::uint8_t, 8> payload {};
    payload[0] = 1;
    ASSERT_TRUE(per_fw_dispatch_can(per_fw_start_button_id(), payload.data(), 1, 1234));
    ASSERT_TRUE(per_fw_start_button_pressed());
    ASSERT_EQ(per_fw_start_button_last_rx(), 1234U);

    payload[0] = 0;
    ASSERT_TRUE(per_fw_dispatch_can(UINT32_MAX, payload.data(), 1, 2000));
    EXPECT_TRUE(per_fw_start_button_pressed());
    EXPECT_EQ(per_fw_start_button_last_rx(), 1234U);
}
