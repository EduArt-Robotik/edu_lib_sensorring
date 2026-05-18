// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SchedulerTypes.hpp
 * @author EduArt Robotik GmbH
 * @brief  Types and helpers for the tick-based measurement scheduler.
 * @date   2026-05-04
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "sensorring/device/DeviceType.hpp"

namespace eduart {

namespace sensorring {

namespace manager {

/**
 * @struct SensorGroupSchedule
 * @brief Scheduling parameters for one group of sensors (same DeviceType).
 *
 * All sensors of the same type fire together (global shutter per group).
 * The divisor determines how many base ticks pass between measurements.
 */
struct SensorGroupSchedule {
  /// Device type this group represents.
  device::DeviceType type = device::DeviceType::UNDEFINED;

  /// Integer divisor: group fires every N-th base tick.
  unsigned int divisor = 1;

  /// Maximum hardware rate of this group (Hz). Determined by the slowest sensor in the group.
  double max_rate_hz = 0.0;

  /// Effective rate after divisor is applied: base_rate / divisor.
  double effective_rate_hz = 0.0;

  /// A measurement has been requested and data is expected (waiting for completion).
  bool has_pending_request = false;

  /// Data from a completed measurement is ready to be fetched from the devices.
  bool has_fetch_ready = false;
};

/**
 * @brief Compute the base tick rate using the fastest sensor group as reference.
 *
 * The fastest group's target rate (after user caps) becomes the base tick rate.
 * All other groups get integer divisors rounded from base / target.
 *
 * @param[in,out] groups  Groups with max_rate_hz filled in. Divisor and effective_rate_hz are computed.
 * @param[in] cap_tof_hz  User-configured cap for depth sensors (0 = no cap).
 * @param[in] cap_thermal_hz User-configured cap for thermal sensors (0 = no cap).
 * @return Base tick rate in Hz (tenths precision).
 */
inline double computeScheduleFastest(std::vector<SensorGroupSchedule>& groups, double cap_tof_hz, double cap_thermal_hz) {
  if (groups.empty()) {
    return 0.0;
  }

  auto targetRate = [&](const SensorGroupSchedule& g) -> double {
    double rate = g.max_rate_hz;
    if (g.type == device::DeviceType::VL53L8CX && cap_tof_hz > 0.0) {
      rate = std::min(rate, cap_tof_hz);
    }
    if (g.type == device::DeviceType::HTPA32 && cap_thermal_hz > 0.0) {
      rate = std::min(rate, cap_thermal_hz);
    }
    return rate;
  };

  // Find the fastest target rate — this becomes the base tick rate.
  double base_rate = 0.0;
  for (auto& g : groups) {
    base_rate = std::max(base_rate, targetRate(g));
  }

  if (base_rate <= 0.0) {
    return 0.0;
  }

  // Round base_rate to tenths.
  base_rate = std::round(base_rate * 10.0) / 10.0;

  // Compute integer divisors.
  for (auto& g : groups) {
    double target = targetRate(g);
    if (target <= 0.0) {
      g.divisor = 1;
    } else {
      double raw_divisor = base_rate / target;
      g.divisor          = std::max(1u, static_cast<unsigned int>(std::round(raw_divisor)));
    }
    g.effective_rate_hz = base_rate / static_cast<double>(g.divisor);
  }

  return base_rate;
}

/**
 * @brief Compute the base tick rate by finding the best common multiple.
 *
 * Searches for the best base rate ≤ max_tick_hz that minimises the total
 * relative deviation between each group's target rate and its effective rate
 * (base / integer_divisor). Candidates are integer multiples of each target
 * rate — these are the only values that can produce zero error for a group.
 *
 * @param[in,out] groups  Groups with max_rate_hz filled in. Divisor and effective_rate_hz are computed.
 * @param[in] cap_tof_hz  User-configured cap for depth sensors (0 = no cap).
 * @param[in] cap_thermal_hz User-configured cap for thermal sensors (0 = no cap).
 * @param[in] max_tick_hz Maximum allowable base tick rate (default 100 Hz).
 * @return Base tick rate in Hz (tenths precision).
 */
inline double computeScheduleCommonMultiple(std::vector<SensorGroupSchedule>& groups, double cap_tof_hz, double cap_thermal_hz, double max_tick_hz = 100.0) {
  if (groups.empty()) {
    return 0.0;
  }

  auto targetRate = [&](const SensorGroupSchedule& g) -> double {
    double rate = g.max_rate_hz;
    if (g.type == device::DeviceType::VL53L8CX && cap_tof_hz > 0.0) {
      rate = std::min(rate, cap_tof_hz);
    }
    if (g.type == device::DeviceType::HTPA32 && cap_thermal_hz > 0.0) {
      rate = std::min(rate, cap_thermal_hz);
    }
    return rate;
  };

  // Collect target rates for all groups.
  std::vector<double> targets;
  targets.reserve(groups.size());
  for (auto& g : groups) {
    double t = targetRate(g);
    if (t > 0.0) {
      targets.push_back(t);
    }
  }

  if (targets.empty()) {
    return 0.0;
  }

  // Generate candidate base rates: integer multiples of each target rate, up to max_tick_hz.
  std::vector<double> candidates;
  for (double t : targets) {
    for (unsigned int k = 1; k * t <= max_tick_hz + 0.05; ++k) {
      double candidate = std::round(k * t * 10.0) / 10.0;
      if (candidate > 0.0 && candidate <= max_tick_hz) {
        candidates.push_back(candidate);
      }
    }
  }

  if (candidates.empty()) {
    candidates.push_back(std::round(*std::max_element(targets.begin(), targets.end()) * 10.0) / 10.0);
  }

  // Remove duplicates.
  std::sort(candidates.begin(), candidates.end());
  candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

  // Score each candidate: sum of relative errors across all groups.
  auto scoreCandidate = [&](double base) -> double {
    double total_error = 0.0;
    for (double t : targets) {
      unsigned int divisor = std::max(1u, static_cast<unsigned int>(std::round(base / t)));
      double effective     = base / static_cast<double>(divisor);
      double rel_error     = std::abs(effective - t) / t;
      total_error += rel_error;
    }
    return total_error;
  };

  // Find the candidate with lowest total error. On ties, prefer lower base rate.
  double best_rate  = candidates.front();
  double best_score = scoreCandidate(best_rate);

  for (double c : candidates) {
    double score = scoreCandidate(c);
    if (score < best_score - 1e-9) {
      best_score = score;
      best_rate  = c;
    }
  }

  // Assign divisors using the best base rate.
  for (std::size_t i = 0; i < groups.size(); ++i) {
    double target = targetRate(groups[i]);
    if (target <= 0.0) {
      groups[i].divisor = 1;
    } else {
      groups[i].divisor = std::max(1u, static_cast<unsigned int>(std::round(best_rate / target)));
    }
    groups[i].effective_rate_hz = best_rate / static_cast<double>(groups[i].divisor);
  }

  return best_rate;
}

} // namespace manager

} // namespace sensorring

} // namespace eduart
