// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Rate.hpp
 * @author EduArt Robotik GmbH
 * @brief  Thread-safe rate measurement utility for tracking callback frequency and sensor count
 * @date 2025-11-18
 */

#pragma once

#include <chrono>
#include <mutex>

namespace eduart {

namespace sensorring {

struct Rate {
  using Clock     = std::chrono::steady_clock;
  using Duration  = Clock::duration;
  using TimePoint = Clock::time_point;
  using toSeconds = std::chrono::duration<double>;

  void tick(unsigned int sensor_count = 0) {
    std::lock_guard<std::mutex> lock(mutex);
    init_flag = true;
    duration += Clock::now() - last_measurement;
    last_measurement = Clock::now();
    counter++;
    this->sensor_count = sensor_count;
  }

  double getRate() {
    std::lock_guard<std::mutex> lock(mutex);
    if (init_flag) {
      auto rate = static_cast<double>(counter) / toSeconds(duration).count();
      duration  = Duration::zero();
      counter   = 0;
      return rate;
    }
    return 0.0;
  }

  unsigned int getSensorCount() {
    std::lock_guard<std::mutex> lock(mutex);
    return sensor_count;
  }

  bool gotFirstMeasurement() {
    std::lock_guard<std::mutex> lock(mutex);
    return init_flag;
  }

private:
  std::mutex mutex;
  bool init_flag             = false;
  unsigned int counter       = 0;
  unsigned int sensor_count  = 0;
  Duration duration          = Duration::zero();
  TimePoint last_measurement = Clock::time_point::min();
};

} // namespace sensorring

} // namespace eduart