#pragma once

#include <chrono>
#include <cstdint>

#include "sensorring/device/DeviceID.hpp"

namespace eduart {

namespace measurement {

/**
 * @brief Generic measurement envelope: metadata + typed payload.
 * @tparam T Payload type (e.g. std::vector<TofMeasurement>, std::vector<ThermalMeasurement>).
 */
template <typename T> struct SENSORRING_EXPORT Measurement {
  
  struct SENSORRING_EXPORT Metadata {
    device::DeviceID id;
    std::chrono::steady_clock::time_point timestamp{};
    std::uint64_t sequence_id{ 0 };
  };
  
  /// Metadata common to all measurement types (timestamp, sequence, device).
  Metadata header;

  /// Typed payload.
  T data;
};

} // namespace measurement

} // namespace eduart
