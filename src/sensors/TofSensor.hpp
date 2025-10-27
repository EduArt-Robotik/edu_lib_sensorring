#pragma once

#include <utility>
#include <vector>
#include "interface/ComInterface.hpp"

#include "BaseSensor.hpp"
#include "CustomTypes.hpp"
#include "Math.hpp"
#include "Parameters.hpp"

#define TOF_RESOLUTION 64

namespace eduart {

namespace sensor {

class TofSensor : public BaseSensor {
public:
  TofSensor(TofSensorParams params, com::ComInterface* interface, std::size_t idx);
  ~TofSensor();

  const TofSensorParams& getParams() const;
  std::pair<measurement::TofMeasurement, SensorState> getLatestRawMeasurement() const;
  std::pair<measurement::TofMeasurement, SensorState> getLatestTransformedMeasurement() const;

  void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;
  void onClearDataFlag() override;

  static void requestTofMeasurement(com::ComInterface* interface, std::uint16_t active_sensors);
  static void fetchTofMeasurement(com::ComInterface* interface, std::uint16_t active_sensors);

  static measurement::TofMeasurement transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);

private:
  // ToDo: Use BoardManager
  // clang-format off
  static constexpr double lut_tan_x[] = {
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624,
      0.3624,0.2589,0.1553,0.0518,-0.0518,-0.1553,-0.2589,-0.3624
  };

  static constexpr double lut_tan_y[] = {
      0.3624, 0.3624, 0.3624, 0.3624, 0.3624, 0.3624, 0.3624, 0.3624,
      0.2589, 0.2589, 0.2589, 0.2589, 0.2589, 0.2589, 0.2589, 0.2589,
      0.1553, 0.1553, 0.1553, 0.1553, 0.1553, 0.1553, 0.1553, 0.1553,
      0.0518, 0.0518, 0.0518, 0.0518, 0.0518, 0.0518, 0.0518, 0.0518,
      -0.0518, -0.0518, -0.0518, -0.0518, -0.0518, -0.0518, -0.0518, -0.0518,
      -0.1553, -0.1553, -0.1553, -0.1553, -0.1553, -0.1553, -0.1553, -0.1553,
      -0.2589, -0.2589, -0.2589, -0.2589, -0.2589, -0.2589, -0.2589, -0.2589,
      -0.3624, -0.3624, -0.3624, -0.3624, -0.3624, -0.3624, -0.3624, -0.3624
  };
  // clang-format on

  measurement::TofMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

  const TofSensorParams _params;
  measurement::TofMeasurement _latest_raw_measurement;
  measurement::TofMeasurement _latest_transformed_measurement;

  uint8_t _rx_buffer[TOF_RESOLUTION * 3];
  unsigned int _rx_buffer_offset;
};

} // namespace sensor

} // namespace eduart