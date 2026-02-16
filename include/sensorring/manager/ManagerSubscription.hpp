#pragma once

#include <cstdint>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace manager {

/**
 * @brief Unique token to identify device-group subscriptions.
 */
using SENSORRING_EXPORT SubscriptionToken = std::uint64_t;

/**
 * @brief Identifies which device group a subscription is for.
 */
enum class SENSORRING_EXPORT DeviceGroupKey {
  ToF,
  Thermal,
  Light
};

} // namespace manager

} // namespace eduart