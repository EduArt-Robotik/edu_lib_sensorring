#pragma once

#include <cstdint>
#include <future>
#include <mutex>
#include <vector>

#include "sensorring/device/BaseSensor.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/TofMeasurement.hpp"

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

  std::pair<const measurement::TofMeasurement&, DeviceState> getLatestMeasurement() const;
  std::pair<const measurement::TofMeasurement&, DeviceState> getLatestTransformedMeasurement() const;

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

  void onResetSensorState();
  void onClearDataFlag();

  static measurement::TofMeasurement transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);

private:
  measurement::TofMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  VL53L8CX_Device& _parent;

  const VL53L8CX_Params _params;
  measurement::TofMeasurement _latest_raw_measurement;
  measurement::TofMeasurement _latest_transformed_measurement;

  uint8_t _rx_buffer[vl53l8::TOF_RESOLUTION * 3];
  std::size_t _rx_buffer_offset = 0;
  bool _rx_buffer_complete      = false;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
