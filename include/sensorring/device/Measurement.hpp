#pragma once

#include <chrono>
#include <cstdint>
#include <variant>
#include <vector>

#include "sensorring/device/DeviceID.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"
#include "sensorring/types/TofMeasurement.hpp"

namespace eduart {

namespace device {

template <typename PayloadT> struct Message {
  // Metadata
  DeviceID id;
  std::chrono::system_clock::time_point timestamp;
  double value;

  // Payload
  PayloadT payload;
};

using VL53L8Payload  = measurement::TofMeasurement;
using HTPA32Payload  = measurement::ThermalMeasurement;
using WS2812bPayload = std::vector<uint8_t>;

using VL53L8Message  = Message<VL53L8Payload>;
using HTPA32Message  = Message<HTPA32Payload>;
using WS2812bMessage = Message<WS2812bPayload>;

using MessageVariant = std::variant<VL53L8Message, HTPA32Message, WS2812bMessage>;

} // namespace device

} // namespace eduart