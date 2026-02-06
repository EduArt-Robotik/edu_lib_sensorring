#pragma once

#include <stdexcept>
#include <string>
#include <typeindex>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

/**
 * @class CapabilityNotSupported
 * @brief Exception thrown when a requested capability type is not supported.
 */
struct SENSORRING_EXPORT CapabilityNotSupported : std::runtime_error {
  std::type_index capability_type;
  CapabilityNotSupported(std::type_index cap)
      : std::runtime_error(std::string("Capability not supported: ") + cap.name())
      , capability_type(cap) {}
};

} // namespace device

} // namespace eduart