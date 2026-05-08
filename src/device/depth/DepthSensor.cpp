#include "sensorring/device/depth/DepthSensor.hpp"

#include <cmath>

#include "sensorring/math/Math.hpp"

namespace eduart {

namespace sensorring {

namespace device {

DepthSensor::DepthSensor(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y)
    : _fov_x_deg(fov_x_deg)
    , _fov_y_deg(fov_y_deg)
    , _resolution_x(res_x)
    , _resolution_y(res_y) {
  createLookupTable(_fov_x_deg, _fov_y_deg, _resolution_x, _resolution_y, _lut_x, _lut_y);
}

void DepthSensor::createLookupTable(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y, std::vector<double>& lut_x, std::vector<double>& lut_y) {
  lut_x.resize(res_x);
  lut_y.resize(res_y);

  auto fov_x_rad = math::degreesToRadians(fov_x_deg);
  auto fov_y_rad = math::degreesToRadians(fov_y_deg);

  for (unsigned int i = 0; i < res_x; ++i) {
    double angle_x = (0.5 - ((static_cast<double>(i) + 0.5) / res_x)) * fov_x_rad;
    lut_x[i]       = std::tan(angle_x);
  }

  for (unsigned int j = 0; j < res_y; ++j) {
    double angle_y = (0.5 - ((static_cast<double>(j) + 0.5) / res_y)) * fov_y_rad;
    lut_y[j]       = std::tan(angle_y);
  }
}

void DepthSensor::processRawMeasurement(const std::vector<double>& lut_x, const std::vector<double>& lut_y, measurement::PointCloud& pcl) {

  unsigned int i = 0;
  for (const auto lut_val_x : lut_x) {
    for (const auto lut_val_y : lut_y) {

      const auto& raw_distance = pcl.data[i].raw_distance;
      if (raw_distance > 0) {
        double x = raw_distance * lut_val_x;
        double y = raw_distance * lut_val_y;
        double z = raw_distance;
        pcl.data[i].point = math::Vector3{ { x, y, z } };
      } else {
        pcl.data[i].point = math::Vector3{ { 0.0, 0.0, 0.0 } };
        pcl.data[i].raw_distance = -1.0;
        pcl.data[i].sigma = -1.0;
      }

      i++;
    }
  }
}

} // namespace device

} // namespace sensorring

} // namespace eduart