#pragma once

#include <cstdint>
#include <future>
#include <mutex>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"

#include "TMF8829_Constants.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
} // namespace com

namespace device {

class TMF8829_Device;

/**
 * @class TMF8829_DeviceImpl
 * @brief Implementation class for TMF8829_Device hiding all private members.
 */
class TMF8829_DeviceImpl {
public:
  TMF8829_DeviceImpl(TMF8829_Device& parent, TMF8829_Params params, com::ComInterface* interface, unsigned int idx);
  ~TMF8829_DeviceImpl();

  const TMF8829_Params& getParams() const;

  std::pair<const measurement::DepthMeasurement&, DeviceState> getLatestMeasurement() const;
  std::pair<const measurement::DepthMeasurement&, DeviceState> getLatestTransformedMeasurement() const;

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

  static measurement::DepthMeasurement transformMeasurement(const measurement::DepthMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);

private:
  measurement::DepthMeasurement processMeasurement(const std::vector<uint8_t>& data) const;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  TMF8829_Device& _parent;

  const TMF8829_Params _params;
  measurement::DepthMeasurement _latest_raw_measurement;
  measurement::DepthMeasurement _latest_transformed_measurement;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
