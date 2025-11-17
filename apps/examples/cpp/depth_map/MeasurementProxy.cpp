#include "MeasurementProxy.hpp"

#include <algorithm>
#include <iostream>

namespace eduart {

double MeasurementProxy::getRate() {
  auto now  = Clock::now();
  auto rate = static_cast<double>(_counter) / toSeconds(now - _lastQuery).count();

  _lastQuery = now;
  _counter   = 0;

  return rate;
}

bool MeasurementProxy::gotFirstMeasurement() {
  return _init_flag;
}

void MeasurementProxy::onRawTofMeasurement([[maybe_unused]] std::vector<measurement::TofMeasurement> measurement_vec) {
  _init_flag = true;
  _counter++;

  printDepthMap(measurement_vec.at(0).point_cloud);
}

void MeasurementProxy::onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] std::string msg) {
  if (verbosity > logger::LogVerbosity::Debug) {
    std::cout << "[" << verbosity << "] " << msg << std::endl;
    std::cout << "\033[s";
  }
}

std::string MeasurementProxy::depthToColor(double depth, double min, double max) {
  // Clamp to range
  if (depth < min)
    depth = min;
  if (depth > max)
    depth = max;

  auto t = (depth - min) / (max - min);

  // Map t to a color gradient: red (near) → yellow → green → blue (far)
  int r = static_cast<int>(255 * (1 - t));
  int g = static_cast<int>(255 * (1 - std::abs(0.5f - t) * 2));
  int b = static_cast<int>(255 * t);

  return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

void MeasurementProxy::printDepthMap(const measurement::PointCloud& points) {

  std::cout << "\033[u";

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      int idx = row * 8 + col;
      std::cout << depthToColor(points[idx].raw_distance, MIN_DIST, MAX_DIST) << "██";
    }
    std::cout << "\033[0m\n";
  }

  std::cout.flush();
}

} // namespace eduart