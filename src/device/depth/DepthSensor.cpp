#include "sensorring/device/depth/DepthSensor.hpp"

#include <cmath>

#include "sensorring/logger/Logger.hpp"
#include "sensorring/math/Math.hpp"

namespace eduart {

namespace sensorring {

namespace device {

DepthSensor::DepthSensor(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable, Config config, double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y)
    : Sensor(id, interface, target, enable)
    , _config(config)
    , _fov_x_deg(fov_x_deg)
    , _fov_y_deg(fov_y_deg)
    , _resolution_x(res_x)
    , _resolution_y(res_y) {

  createLookupTable(_fov_x_deg, _fov_y_deg, _resolution_x, _resolution_y, _lut_x, _lut_y, _lut_z);
}

void DepthSensor::updateResolution(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y) {
  _fov_x_deg    = fov_x_deg;
  _fov_y_deg    = fov_y_deg;
  _resolution_x = res_x;
  _resolution_y = res_y;
  createLookupTable(_fov_x_deg, _fov_y_deg, _resolution_x, _resolution_y, _lut_x, _lut_y, _lut_z);
}

void DepthSensor::createLookupTable(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y, std::vector<double>& lut_x, std::vector<double>& lut_y, std::vector<double>& lut_z) {
  lut_x.resize(res_x);
  lut_y.resize(res_y);

  auto fov_x_rad = math::degreesToRadians(fov_x_deg);
  auto fov_y_rad = math::degreesToRadians(fov_y_deg);

  auto side_length_x = std::tan(fov_x_rad / 2.0);
  auto side_length_y = std::tan(fov_y_rad / 2.0);

  // X axis LUT
  for (unsigned int i = 0; i < res_x; ++i) {
    // Constant angle assumption (slightly incorrect from what we know from the datasheet):
    // auto angle_x = (0.5 - ((static_cast<double>(i) + 0.5) / res_x)) * fov_x_rad;

    // Constant zone length assumption (correct from what we know from the datasheet):
    auto idx_factor        = (i - (res_x / 2.0) + 0.5) / (res_x / 2.0);
    auto corrected_angle_x = std::atan(idx_factor * side_length_x);

    if (_config.invert_x_lut) {
      corrected_angle_x = -corrected_angle_x;
    }

    lut_x[i] = std::tan(corrected_angle_x);
  }

  // Y axis LUT
  for (unsigned int j = 0; j < res_y; ++j) {
    // Constant angle assumption (slightly incorrect from what we know from the datasheet):
    // auto angle_y = (0.5 - ((static_cast<double>(j) + 0.5) / res_y)) * fov_y_rad;

    // Constant zone length assumption (correct from what we know from the datasheet):
    auto idx_factor        = (j - (res_y / 2.0) + 0.5) / (res_y / 2.0);
    auto corrected_angle_y = std::atan(idx_factor * side_length_y);

    if (_config.invert_y_lut) {
      corrected_angle_y = -corrected_angle_y;
    }

    lut_y[j] = std::tan(corrected_angle_y);
  }

  // depth LUT (correction factor if the sensor reports the direct distance (hypotenuse) instead of perpendicular distance)
  if (!_config.reports_perpendicular_distance) {
    lut_z.resize(res_x * res_y);
    for (unsigned int i = 0; i < res_x; ++i) {
      for (unsigned int j = 0; j < res_y; ++j) {
        lut_z[j * res_x + i] = 1 / std::sqrt(1 + lut_x[i] * lut_x[i] + lut_y[j] * lut_y[j]);
      }
    }
  }
}

void DepthSensor::processRawMeasurement(measurement::PointCloud& pcl) {

  if (pcl.data.size() != (_resolution_x * _resolution_y)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Point cloud size does not match sensor resolution");
    return;
  }

  if (_lut_x.size() != _resolution_x || _lut_y.size() != _resolution_y) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Lookup table size does not match sensor resolution");
    return;
  }

  unsigned int i = 0;
  for (const auto lut_val_y : _lut_y) {
    for (const auto lut_val_x : _lut_x) {

      const auto& raw_distance = pcl.data[i].raw_distance;
      if (raw_distance > 0) {

        // Apply distance correction if the sensor reports the direct distance but keep raw distance unchanged
        const auto distance = _config.reports_perpendicular_distance ? raw_distance : raw_distance * _lut_z[i];

        double x          = distance * lut_val_x;
        double y          = distance * lut_val_y;
        double z          = distance;
        pcl.data[i].point = math::Vector3{
          { x, y, z }
        };
      } else {
        pcl.data[i].point = math::Vector3{
          { 0.0, 0.0, 0.0 }
        };
        pcl.data[i].raw_distance = -1.0;
        pcl.data[i].sigma        = -1.0;
      }

      i++;
    }
  }

  const auto idx = getDeviceID().getIndex();
  for (auto& p : pcl.data) {
    p.sensor_index = idx;
  }
}

const measurement::DepthMeasurement& DepthSensor::getLatestMeasurement() const {
  return _latest_measurement;
}

void DepthSensor::publishMeasurement() {
  if (!getEnable())
    return;
  _latest_measurement.header.timestamp = std::chrono::system_clock::now();
  _depth_publisher.publish(_latest_measurement);
}

} // namespace device

} // namespace sensorring

} // namespace eduart