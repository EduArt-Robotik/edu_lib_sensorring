#pragma once

#include <string>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace device {

enum class DeviceType {
  VL53L8,
  HTPA32,
  WS2812b,
  UNDEFINED
};

struct SENSORRING_EXPORT DeviceID {
public:
  DeviceType type = DeviceType::UNDEFINED;
  std::string name = "";
  unsigned int index = 0;

  bool isValid() const;
};

inline bool DeviceID::isValid() const {
  return type != DeviceType::UNDEFINED && name != "";
}

} // namespace device

} // namespace eduart