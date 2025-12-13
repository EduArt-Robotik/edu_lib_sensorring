// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementProxy.cpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class for the depth map C++ example
 * @date 2025-11-18
 */

#include "MeasurementProxy.hpp"

#include <iostream>
#include <sensorring/logger/Logger.hpp>

namespace eduart {

MeasurementProxy::MeasurementProxy() {
  // Register the proxy with the Logger to get the log output
  logger::Logger::getInstance()->registerClient(this);
}

MeasurementProxy::~MeasurementProxy() {
  // Important, otherwise a segfault may occur on program exit
  logger::Logger::getInstance()->unregisterClient(this);
}

bool MeasurementProxy::gotFirstMeasurement() {
  return _init_flag;
}

void MeasurementProxy::onRawTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) {
  _init_flag = true;

  printDepthMap(measurement_vec.at(0).point_cloud);
}

void MeasurementProxy::onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] const std::string& msg) {
  if (verbosity > logger::LogVerbosity::Debug) {
    std::cout << "[" << verbosity << "] " << msg << std::endl;
    _reset_cursor = false;
  }
}

std::string MeasurementProxy::depthToColor(double depth, double min, double max) {

  if (depth > max || depth < 0) {
    depth = max;
  } else if (depth < min) {
    depth = min;
  }

  auto d = (depth - min) / (max - min);

  // Map t to a color gradient: red (near) → yellow → green → blue (far)
  int r = static_cast<int>(255 * (1 - d));
  int g = static_cast<int>(255 * (1 - std::abs(0.5f - d) * 2));
  int b = static_cast<int>(255 * d);

  return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

void MeasurementProxy::printDepthMap(const measurement::PointCloud& points) {

  if (_reset_cursor) {
    std::cout << "\033[8F";
  }

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      int idx = row * 8 + col;
      std::cout << depthToColor(points.data[idx].raw_distance, MIN_DIST, MAX_DIST) << "██";
    }
    std::cout << "\033[0m\n";
  }

  std::cout.flush();
  _reset_cursor = true;
}

} // namespace eduart