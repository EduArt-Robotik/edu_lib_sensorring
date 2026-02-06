#pragma once

#include <utility>
#include <vector>

#include "hardware/st_vl53l8cx.hpp"
#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/IDevice.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/types/TofMeasurement.hpp"

#include "BaseSensor.hpp"

namespace eduart {

namespace device {

struct GetLatestRawMeasurement {
  struct Request {};
  struct Response {
    const measurement::TofMeasurement& measurement;
    SensorState state;
  };
};

struct GetLatestTransformedMeasurement {
  struct Request {};
  struct Response {
    const measurement::TofMeasurement& measurement;
    SensorState state;
  };
};

struct RequestTofMeasurement {
  struct Request {
    com::ComInterface* interface;
    unsigned int active_sensors;
  };
  struct Response {};
};

struct FetchTofMeasurement {
  struct Request {
    com::ComInterface* interface;
    unsigned int active_sensors;
  };
  struct Response {};
};

class TofSensor : public BaseSensor, public IDevice, ICapability<GetLatestRawMeasurement>, ICapability<GetLatestTransformedMeasurement> {
public:
  TofSensor(TofSensorParams params, com::ComInterface* interface, std::size_t idx);
  ~TofSensor();

  const TofSensorParams& getParams() const;

  GetLatestRawMeasurement::Response invoke(const GetLatestRawMeasurement::Request&) const override;
  GetLatestTransformedMeasurement::Response invoke(const GetLatestTransformedMeasurement::Request&) const override;

  static RequestTofMeasurement::Response requestTofMeasurement(const RequestTofMeasurement::Request& req);
  static FetchTofMeasurement::Response fetchTofMeasurement(const FetchTofMeasurement::Request& req);

  void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  void onResetSensorState() override;
  void onClearDataFlag() override;

  static measurement::TofMeasurement transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation);
  measurement::TofMeasurement processMeasurement(int frame_id, uint8_t* data, int len) const;

  const TofSensorParams _params;
  measurement::TofMeasurement _latest_raw_measurement;
  measurement::TofMeasurement _latest_transformed_measurement;

  static constexpr unsigned int MAX_SENSOR_SELECT_SIZE = 16;

  uint8_t _rx_buffer[vl53l8::TOF_RESOLUTION * 3];
  std::size_t _rx_buffer_offset;
};

} // namespace device

} // namespace eduart