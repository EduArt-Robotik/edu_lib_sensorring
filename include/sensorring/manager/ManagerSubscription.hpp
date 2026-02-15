#pragma once

#include <cstdint>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

using SENSORRING_EXPORT SubscriptionToken = std::uint64_t;

enum class SENSORRING_EXPORT DeviceGroupKey {
  ToF,
  Thermal,
  Light
};

} // namespace manager

} // namespace eduart