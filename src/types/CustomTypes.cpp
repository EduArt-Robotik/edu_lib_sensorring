#include "sensorring/types/CustomTypes.hpp"

#include <algorithm>

namespace eduart {

namespace measurement {

// Explicit template instantiation for the used types
template struct GenericGrayscaleImage<uint8_t, THERMAL_RESOLUTION>;
template struct GenericGrayscaleImage<double, THERMAL_RESOLUTION>;

template <typename T, std::size_t RESOLUTION> double GenericGrayscaleImage<T, RESOLUTION>::avg() {
  double result = 0;
  std::size_t i = 0;
  for (auto& element : data) {
    result += static_cast<double>(element);
    i++;
  }
  result /= i;
  return result;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::round() {
  for (auto& element : data) {
    element = static_cast<T>(std::round(element));
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator/=(const T other) {
  for (auto& element : data) {
    element = static_cast<T>(static_cast<double>(element) / static_cast<double>(other));
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+=(const T other) {
  for (auto& element : data) {
    element += other;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+=(const GenericGrayscaleImage<T, RESOLUTION>& other) {
  std::size_t i = 0;
  for (auto& element : data) {
    element += other.data[i];
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> template <typename U> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+=(const GenericGrayscaleImage<U, RESOLUTION>& other) {
  for (std::size_t i = 0; i < RESOLUTION; ++i) {
    data[i] += static_cast<T>(other.data[i]);
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator-=(const T other) {
  for (auto& element : data) {
    element -= other;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator-=(const GenericGrayscaleImage<T, RESOLUTION>& other) {
  std::size_t i = 0;
  for (auto& element : data) {
    element -= other.data[i];
    i++;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> template <typename U> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator-=(const GenericGrayscaleImage<U, RESOLUTION>& other) {
  for (std::size_t i = 0; i < RESOLUTION; ++i) {
    data[i] -= static_cast<T>(other.data[i]);
  }
  return *this;
}

void copyPointCloudTo(const PointCloud& pc, double* buffer, int size) {

  static constexpr size_t STRIDE = 6; // 6 doubles per PointData point
  const size_t max_points        = size / STRIDE;
  const size_t count             = std::min(pc.size(), max_points);

  // double* data = reinterpret_cast<double*>(buffer);
  for (size_t i = 0; i < count; ++i) {
    const auto& p     = pc[i];
    buffer[i * 6 + 0] = p.point.x();
    buffer[i * 6 + 1] = p.point.y();
    buffer[i * 6 + 2] = p.point.z();
    buffer[i * 6 + 3] = p.raw_distance;
    buffer[i * 6 + 4] = p.sigma;
    buffer[i * 6 + 5] = (double)p.user_idx;
  }
}

} // namespace measurement

} // namespace eduart