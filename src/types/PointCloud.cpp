#include "sensorring/types/PointCloud.hpp"

#include <algorithm>

namespace eduart {

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
    buffer[i * 6 + 5] = (double)p.user_idx;
  }
}

} // namespace measurement

} // namespace eduart