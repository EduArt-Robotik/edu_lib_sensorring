#pragma once

#include <cstdint>
#include <string_view>

#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"

namespace eduart {

namespace device {

namespace vl53l8 {

static constexpr std::uint8_t TOF_RESOLUTION = 64;

// Device parameters
static constexpr std::string_view NAME = "ST VL53L8CX";
static constexpr double FOV_X = 45.0;
static constexpr double FOV_Y = 45.0;
static constexpr int RES_X = 8;
static constexpr int RES_Y = 8;
static constexpr double MAX_RATE = 15.0;

// clang-format off
static const double lut_tan_x[] = {
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
    0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624
};

static const double lut_tan_y[] = {
    0.3624, 0.3624, 0.3624, 0.3624, 0.3624, 0.3624, 0.3624, 0.3624,
    0.2589, 0.2589, 0.2589, 0.2589, 0.2589, 0.2589, 0.2589, 0.2589,
    0.1553, 0.1553, 0.1553, 0.1553, 0.1553, 0.1553, 0.1553, 0.1553,
    0.0518, 0.0518, 0.0518, 0.0518, 0.0518, 0.0518, 0.0518, 0.0518,
    -0.0518, -0.0518, -0.0518, -0.0518, -0.0518, -0.0518, -0.0518, -0.0518,
    -0.1553, -0.1553, -0.1553, -0.1553, -0.1553, -0.1553, -0.1553, -0.1553,
    -0.2589, -0.2589, -0.2589, -0.2589, -0.2589, -0.2589, -0.2589, -0.2589,
    -0.3624, -0.3624, -0.3624, -0.3624, -0.3624, -0.3624, -0.3624, -0.3624
};
// clang-format on

} // namespace vl53l8

} // namespace device

} // namespace eduart
