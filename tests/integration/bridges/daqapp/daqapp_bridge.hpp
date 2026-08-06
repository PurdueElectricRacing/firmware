#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {

struct PerCanFrame {
    std::uint32_t id;
    std::uint8_t is_extended;
    std::uint8_t length;
    std::uint8_t data[8];
};

bool per_daq_parse_udp_frame(const std::uint8_t *bytes, std::size_t length, PerCanFrame *output);
bool per_daq_decode_main_hb(
    const char *dbc_path,
    const PerCanFrame *frame,
    double *car_state
);
bool per_daq_encode_start_button(
    const char *dbc_path,
    bool pressed,
    PerCanFrame *output
);

}
