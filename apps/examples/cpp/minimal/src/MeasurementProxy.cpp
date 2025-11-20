// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   MeasurementProxy.cpp
 * @author EduArt Robotik GmbH
 * @brief  Proxy class for the minimal C++ example
 * @date 2025-11-18
 */

#include "MeasurementProxy.hpp"

#include <iostream>

#include "sensorring/logger/LoggerClient.hpp"

namespace eduart {

double MeasurementProxy::getRate() {
  auto rate = static_cast<double>(_counter) / toSeconds(_duration).count();
  _duration = Duration::zero();
  _counter  = 0;
  return rate;
}

bool MeasurementProxy::gotFirstMeasurement() {
  return _init_flag;
}

void MeasurementProxy::onRawTofMeasurement([[maybe_unused]] const std::vector<measurement::TofMeasurement>& measurement_vec) {
  _duration += Clock::now() - _last_measurement;
  _last_measurement = Clock::now();
  _init_flag        = true;
  _counter++;
}

void MeasurementProxy::onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] const std::string& msg) {
  if (verbosity > logger::LogVerbosity::Debug) {
    std::cout << "[" << verbosity << "] " << msg << std::endl;
  }
}

} // namespace eduart