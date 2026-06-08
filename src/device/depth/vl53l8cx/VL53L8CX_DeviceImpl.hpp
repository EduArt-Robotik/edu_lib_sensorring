#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
} // namespace com

namespace device {

class VL53L8CX_Device;

/**
 * @class VL53L8CX_DeviceImpl
 * @brief Implementation class for VL53L8CX_Device hiding all private members.
 */
class VL53L8CX_DeviceImpl {
public:
  VL53L8CX_DeviceImpl(VL53L8CX_Device& parent, VL53L8CX_Params params, com::ComInterface* interface, unsigned int idx);
  ~VL53L8CX_DeviceImpl();

  const VL53L8CX_Params& getParams() const;

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

private:
  measurement::DepthMeasurement processMeasurement(const std::vector<uint8_t>& data) const;

  static constexpr unsigned int RESOLUTION = 64;

  VL53L8CX_Device& _parent;

  const VL53L8CX_Params _params;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
