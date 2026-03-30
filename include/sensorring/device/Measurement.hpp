// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Measurement.hpp
 * @author EduArt Robotik GmbH
 * @brief  Message and measurement type definitions.
 * @date   2026-02-19
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <variant>
#include <vector>

#include "sensorring/device/DeviceID.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"
#include "sensorring/measurement/TofMeasurement.hpp"

namespace eduart {

namespace device {

/**
 * @struct Message
 * @brief Generic message container with metadata and payload.
 * @tparam PayloadT The type of data carried in the message.
 */
template <typename PayloadT> struct Message {
  // Metadata
  /// The device that generated this message.
  DeviceID id;

  /// Timestamp when the measurement was taken.
  std::chrono::system_clock::time_point timestamp;

  /// Associated numeric value (e.g., measurement quality, confidence).
  double value;

  // Payload
  /// The measurement data payload.
  PayloadT payload;
};

using VL53L8Payload  = measurement::TofMeasurement;     ///< Payload type for VL53L8CX ToF sensor measurements.
using HTPA32Payload  = measurement::ThermalMeasurement; ///< Payload type for HTPA32 thermal camera measurements.
using WS2812bPayload = std::vector<uint8_t>;            ///< Payload type for WS2812b LED control messages.

using VL53L8Message  = Message<VL53L8Payload>;  ///< Complete message from VL53L8CX ToF sensor.
using HTPA32Message  = Message<HTPA32Payload>;  ///< Complete message from HTPA32 thermal camera.
using WS2812bMessage = Message<WS2812bPayload>; ///< Complete message for WS2812b LED control.

using MessageVariant = std::variant<VL53L8Message, HTPA32Message, WS2812bMessage>; ///< Variant holding any measurement message type.

} // namespace device

} // namespace eduart