// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Rate.cpp
 * @author EduArt Robotik GmbH
 * @brief  Thread-safe rate measurement utility for tracking callback frequency and sensor count
 * @date 2025-11-18
 */

#include "Rate.hpp"

namespace eduart {

void Rate::tick(unsigned int sensor_count) {
  std::lock_guard<std::mutex> lock(mutex);
  init_flag = true;
  duration += Clock::now() - last_measurement;
  last_measurement = Clock::now();
  counter++;
  this->sensor_count = sensor_count;
}

double Rate::getRate() {
  std::lock_guard<std::mutex> lock(mutex);
  if (init_flag) {
    auto rate = static_cast<double>(counter) / toSeconds(duration).count();
    duration  = Duration::zero();
    counter   = 0;
    return rate;
  }
  return 0.0;
}

unsigned int Rate::getSensorCount() {
  std::lock_guard<std::mutex> lock(mutex);
  return sensor_count;
}

bool Rate::gotFirstMeasurement() {
  std::lock_guard<std::mutex> lock(mutex);
  return init_flag;
}

} // namespace eduart
