#pragma once

#include <chrono>
#include <condition_variable>
#include <future>
#include <vector>

#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/types/TofMeasurement.hpp"

#include "st_vl53l8cx.hpp"

namespace eduart {

namespace device {

struct VL53L8CX_Device : BaseDevice {
public:
  VL53L8CX_Device(VL53L8CX_Params params, com::ComInterface* interface, unsigned int idx);
  ~VL53L8CX_Device();

  const VL53L8CX_Params& getParams() const;

  std::pair<const measurement::TofMeasurement&, SensorState> getLatestRawMeasurement() const;
  std::pair<const measurement::TofMeasurement&, SensorState> getLatestTransformedMeasurement() const;
  std::future<bool> requestTofMeasurementAsync(std::chrono::milliseconds timeout);
  std::future<bool> fetchTofMeasurementAsync(std::chrono::milliseconds timeout);

  static std::future<bool> requestTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout);
  static std::future<bool> fetchTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout);

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  static measurement::TofMeasurement transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);
  measurement::TofMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

  const VL53L8CX_Params _params;
  measurement::TofMeasurement _latest_raw_measurement;
  measurement::TofMeasurement _latest_transformed_measurement;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  uint8_t _rx_buffer[vl53l8::TOF_RESOLUTION * 3];
  std::size_t _rx_buffer_offset;
  mutable std::condition_variable _data_condition;
};

} // namespace device

} // namespace eduart
