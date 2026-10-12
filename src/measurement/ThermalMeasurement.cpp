#include "sensorring/measurement/ThermalMeasurement.hpp"

#include <algorithm>
#include <cmath>

#include "utils/Iron.hpp"

namespace eduart {

namespace sensorring {

namespace measurement {

GrayscaleImage TemperatureImage::toGrayscale(double t_min, double t_max) const {
  GrayscaleImage result;
  double delta = t_max - t_min;
  if (delta == 0.0) {
    return result;
  }
  for (std::size_t i = 0; i < data.size(); ++i) {
    double norm    = ((data[i] - t_min) / delta) * 255.0;
    result.data[i] = static_cast<std::uint8_t>(std::clamp(std::round(norm), 0.0, 255.0));
  }
  return result;
}

GrayscaleImage TemperatureImage::toGrayscale() const {
  double t_min = *std::min_element(data.begin(), data.end());
  double t_max = *std::max_element(data.begin(), data.end());
  return toGrayscale(t_min, t_max);
}

FalseColorImage TemperatureImage::toFalseColor(double t_min, double t_max) const {
  GrayscaleImage gray = toGrayscale(t_min, t_max);
  FalseColorImage result;
  for (std::size_t i = 0; i < gray.data.size(); ++i) {
    result.data[i][0] = ThermalPalette::Iron[gray.data[i]][0];
    result.data[i][1] = ThermalPalette::Iron[gray.data[i]][1];
    result.data[i][2] = ThermalPalette::Iron[gray.data[i]][2];
  }
  return result;
}

FalseColorImage TemperatureImage::toFalseColor() const {
  double t_min = *std::min_element(data.begin(), data.end());
  double t_max = *std::max_element(data.begin(), data.end());
  return toFalseColor(t_min, t_max);
}

} // namespace measurement

} // namespace sensorring

} // namespace eduart
