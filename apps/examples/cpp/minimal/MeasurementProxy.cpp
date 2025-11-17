#include "MeasurementProxy.hpp"

#include <iostream>

#include "sensorring/logger/LoggerClient.hpp"

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
}

void MeasurementProxy::onOutputLog([[maybe_unused]] logger::LogVerbosity verbosity, [[maybe_unused]] std::string msg) {
  if (verbosity > logger::LogVerbosity::Debug) {
    std::cout << "[" << verbosity << "] " << msg << std::endl;
  }
}

} // namespace eduart