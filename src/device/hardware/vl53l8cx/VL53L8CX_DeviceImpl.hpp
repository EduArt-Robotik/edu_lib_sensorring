#pragma once

#include <cstdint>
#include <future>
#include <mutex>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"

#include "VL53L8CX_Constants.hpp"

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

  std::pair<const measurement::DepthMeasurement&, DeviceState> getLatestMeasurement() const;
  std::pair<const measurement::DepthMeasurement&, DeviceState> getLatestTransformedMeasurement() const;

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

  static measurement::DepthMeasurement transformMeasurement(const measurement::DepthMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);

private:
  measurement::DepthMeasurement processMeasurement(const std::vector<uint8_t>& data) const;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  VL53L8CX_Device& _parent;

  const VL53L8CX_Params _params;
  measurement::DepthMeasurement _latest_raw_measurement;
  measurement::DepthMeasurement _latest_transformed_measurement;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
