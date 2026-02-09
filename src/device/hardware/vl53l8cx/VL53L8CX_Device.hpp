#pragma once

#include <chrono>
#include <future>
#include <vector>

#include "interface/ComInterface.hpp"
#include "sensorring/Parameter.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/ICapabilityAsync.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/types/TofMeasurement.hpp"

#include "st_vl53l8cx.hpp"

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
    std::chrono::milliseconds timeout{ 1000 };
  };
  struct Response {
    bool ready{ false };
  };
};

struct FetchTofMeasurement {
  struct Request {
    std::chrono::milliseconds timeout{ 1000 };
  };
  struct Response {
    bool complete{ false };
  };
};

struct VL53L8CX_Device : BaseDevice, ICapability<GetLatestRawMeasurement>, ICapability<GetLatestTransformedMeasurement>, ICapabilityAsync<RequestTofMeasurement>, ICapabilityAsync<FetchTofMeasurement> {
public:
  VL53L8CX_Device(VL53L8CX_Params params, com::ComInterface* interface, unsigned int idx);
  ~VL53L8CX_Device();

  const VL53L8CX_Params& getParams() const;

  GetLatestRawMeasurement::Response invoke(const GetLatestRawMeasurement::Request&) const override;
  GetLatestTransformedMeasurement::Response invoke(const GetLatestTransformedMeasurement::Request&) const override;
  std::future<RequestTofMeasurement::Response> invoke_async(const RequestTofMeasurement::Request& req) override;
  std::future<FetchTofMeasurement::Response> invoke_async(const FetchTofMeasurement::Request& req) override;

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
};

} // namespace device

} // namespace eduart
