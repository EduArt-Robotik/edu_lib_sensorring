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

struct Rate {
  using Clock     = std::chrono::steady_clock;
  using Duration  = Clock::duration;
  using TimePoint = Clock::time_point;
  using toSeconds = std::chrono::duration<double>;

  void tick(unsigned int sensor_count = 0);
  double getRate();
  unsigned int getSensorCount();
  bool gotFirstMeasurement();

private:
  std::mutex mutex;
  bool init_flag             = false;
  unsigned int counter       = 0;
  unsigned int sensor_count  = 0;
  Duration duration          = Duration::zero();
  TimePoint last_measurement = Clock::time_point::min();
};

} // namespace eduart
