#pragma once

#include <utility>
#include <vector>

#include "hardware/st_vl53l8cx.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/types/ToFMeasurement.hpp"

#include "BaseSensor.hpp"

namespace eduart {

namespace sensor {

class TofSensor : public BaseSensor {
public:
  TofSensor(TofSensorParams params, com::ComInterface* interface, std::size_t idx);
  ~TofSensor();

  const TofSensorParams& getParams() const;
  std::pair<const measurement::TofMeasurement&, SensorState> getLatestRawMeasurement() const;
  std::pair<const measurement::TofMeasurement&, SensorState> getLatestTransformedMeasurement() const;

  void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

  static void cmdRequestTofMeasurement(com::ComInterface* interface, std::uint16_t active_sensors);
  static void cmdFetchTofMeasurement(com::ComInterface* interface, std::uint16_t active_sensors);

  static measurement::TofMeasurement transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  measurement::TofMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

  const TofSensorParams _params;
  measurement::TofMeasurement _latest_raw_measurement;
  measurement::TofMeasurement _latest_transformed_measurement;

  uint8_t _rx_buffer[vl53l8::TOF_RESOLUTION * 3];
  std::size_t _rx_buffer_offset;
};

} // namespace sensor

} // namespace eduart