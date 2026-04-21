#include "sensorring/measurement/DepthMeasurement.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

PointCloud DepthMeasurement::combinePointClouds(const std::vector<DepthMeasurement>& measurements) {
  std::vector<PointCloud> clouds;
  clouds.reserve(measurements.size());
  for (const auto& m : measurements) {
    clouds.push_back(m.transformed_point_cloud);
  }
  return PointCloud::combine(clouds);
}

} // namespace measurement

} // namespace sensorring

} // namespace eduart
