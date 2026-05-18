#include "sensorring/measurement/Image.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace eduart {

namespace sensorring {

namespace measurement {

// Explicit template instantiation for the used types
template struct ScalarImage<std::uint8_t, THERMAL_RESOLUTION>;
template struct ScalarImage<double, THERMAL_RESOLUTION>;
template struct RgbImage<std::uint8_t, THERMAL_RESOLUTION>;

template <typename T, std::size_t RESOLUTION> double ScalarImage<T, RESOLUTION>::avg() {
  double result = 0;
  std::size_t i = 0;
  for (auto& element : data) {
    result += static_cast<double>(element);
    i++;
  }
  result /= i;
  return result;
}

template <typename T, std::size_t RESOLUTION> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::round() {
  for (auto& element : data) {
    element = static_cast<T>(std::round(element));
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator/=(const T other) {
  for (auto& element : data) {
    element = static_cast<T>(static_cast<double>(element) / static_cast<double>(other));
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator+=(const T other) {
  for (auto& element : data) {
    element += other;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator+=(const ScalarImage<T, RESOLUTION>& other) {
  std::size_t i = 0;
  for (auto& element : data) {
    element += other.data[i];
    i++;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> template <typename U> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator+=(const ScalarImage<U, RESOLUTION>& other) {
  for (std::size_t i = 0; i < RESOLUTION; ++i) {
    data[i] += static_cast<T>(other.data[i]);
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator-=(const T other) {
  for (auto& element : data) {
    element -= other;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator-=(const ScalarImage<T, RESOLUTION>& other) {
  std::size_t i = 0;
  for (auto& element : data) {
    element -= other.data[i];
    i++;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> template <typename U> ScalarImage<T, RESOLUTION>& ScalarImage<T, RESOLUTION>::operator-=(const ScalarImage<U, RESOLUTION>& other) {
  for (std::size_t i = 0; i < RESOLUTION; ++i) {
    data[i] -= static_cast<T>(other.data[i]);
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> void ScalarImage<T, RESOLUTION>::copyTo(T* buffer, int size) {
  const std::size_t count = std::min(static_cast<std::size_t>(size), RESOLUTION);
  for (std::size_t i = 0; i < count; ++i) {
    buffer[i] = data[i];
  }
}

template <typename T, std::size_t RESOLUTION> void RgbImage<T, RESOLUTION>::copyTo(T* buffer, int size) {
  static constexpr std::size_t CHANNELS = 3;
  const std::size_t max_pixels          = static_cast<std::size_t>(size) / CHANNELS;
  const std::size_t count               = std::min(max_pixels, RESOLUTION);
  for (std::size_t i = 0; i < count; ++i) {
    buffer[i * CHANNELS + 0] = data[i][0];
    buffer[i * CHANNELS + 1] = data[i][1];
    buffer[i * CHANNELS + 2] = data[i][2];
  }
}

} // namespace measurement

} // namespace sensorring

} // namespace eduart