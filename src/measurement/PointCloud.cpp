#include "sensorring/measurement/PointCloud.hpp"

#include <algorithm>

namespace eduart {

namespace sensorring {

namespace measurement {

void PointCloud::copyTo(double* buffer, int size) {

  static constexpr size_t STRIDE = 6; // 6 doubles per PointData point
  const size_t max_points        = size / STRIDE;
  const size_t count             = std::min(data.size(), max_points);

  // double* data = reinterpret_cast<double*>(buffer);
  for (size_t i = 0; i < count; ++i) {
    const auto& p     = data[i];
    buffer[i * 6 + 0] = p.point.x();
    buffer[i * 6 + 1] = p.point.y();
    buffer[i * 6 + 2] = p.point.z();
    buffer[i * 6 + 3] = p.raw_distance;
    buffer[i * 6 + 4] = p.sigma;
    buffer[i * 6 + 5] = (double)p.sensor_index;
  }
}

PointCloud PointCloud::combine(const std::vector<PointCloud>& clouds) {
  PointCloud result;
  std::size_t total = 0;
  for (const auto& c : clouds) {
    total += c.data.size();
  }
  result.data.reserve(total);
  for (const auto& c : clouds) {
    result.data.insert(result.data.end(), c.data.begin(), c.data.end());
  }
  return result;
}

PointCloud PointCloud::transform(const PointCloud& cloud, const math::Vector3 translation, const math::Vector3 rotation) {
  auto rot_matrix = math::rotMatrixFromEulerDegrees(rotation);
  return transform(cloud, translation, rot_matrix);
}

PointCloud PointCloud::transform(const PointCloud& cloud, const math::Vector3 translation, const math::Matrix3 rotation) {
  PointCloud result = cloud;
  for (size_t i = 0; i < result.data.size(); ++i) {
    result.data[i].point = (rotation * cloud.data[i].point) + translation;
  }
  return result;
}

} // namespace measurement

} // namespace sensorring

} // namespace eduart