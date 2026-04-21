#include "sensorring/measurement/Helpers.hpp"

#include <algorithm>
#include <cmath>

#include "utils/Iron.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

PointCloud combinePointClouds(const std::vector<PointCloud>& clouds) {
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

PointCloud combinePointClouds(const std::vector<DepthMeasurement>& measurements) {
  std::vector<PointCloud> clouds;
  clouds.reserve(measurements.size());
  for (const auto& m : measurements) {
    clouds.push_back(m.transformed_point_cloud);
  }
  return combinePointClouds(clouds);
}

GrayscaleImage toGrayscale(const TemperatureImage& img, double t_min, double t_max) {
  GrayscaleImage result;
  double delta = t_max - t_min;
  if (delta == 0.0) {
    return result;
  }
  for (std::size_t i = 0; i < img.data.size(); ++i) {
    double norm    = ((img.data[i] - t_min) / delta) * 255.0;
    result.data[i] = static_cast<std::uint8_t>(std::clamp(std::round(norm), 0.0, 255.0));
  }
  return result;
}

GrayscaleImage toGrayscale(const TemperatureImage& img) {
  double t_min = *std::min_element(img.data.begin(), img.data.end());
  double t_max = *std::max_element(img.data.begin(), img.data.end());
  return toGrayscale(img, t_min, t_max);
}

FalseColorImage toFalseColor(const TemperatureImage& img, double t_min, double t_max) {
  GrayscaleImage gray = toGrayscale(img, t_min, t_max);
  FalseColorImage result;
  for (std::size_t i = 0; i < gray.data.size(); ++i) {
    result.data[i][0] = ThermalPalette::Iron[gray.data[i]][0];
    result.data[i][1] = ThermalPalette::Iron[gray.data[i]][1];
    result.data[i][2] = ThermalPalette::Iron[gray.data[i]][2];
  }
  return result;
}

FalseColorImage toFalseColor(const TemperatureImage& img) {
  double t_min = *std::min_element(img.data.begin(), img.data.end());
  double t_max = *std::max_element(img.data.begin(), img.data.end());
  return toFalseColor(img, t_min, t_max);
}

} // namespace measurement

} // namespace sensorring

} // namespace eduart
