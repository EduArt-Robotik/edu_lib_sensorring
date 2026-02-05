#pragma once

#include <stdexcept>
#include <string>
#include <typeindex>

struct CapabilityNotSupported : std::runtime_error {
  std::type_index capability_type;
  CapabilityNotSupported(std::type_index cap)
      : std::runtime_error(std::string("Capability not supported: ") + cap.name())
      , capability_type(cap) {}
};