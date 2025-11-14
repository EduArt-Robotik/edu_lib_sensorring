#include "sensorring/utils/CustomTypes.hpp"

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
  std::size_t i = 0;
  for (auto& element : data) {
    element =  static_cast<T>(static_cast<double>(element) / static_cast<double>(other));
    i++;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+=(const T other) {
  std::size_t i = 0;
  for (auto& element : data) {
    element += other;
    i++;
  }
  return *this;
}

template <typename T, std::size_t RESOLUTION> GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+=(const GenericGrayscaleImage<T, RESOLUTION>& other) {
  std::size_t i = 0;
  for (auto& element : data) {
    element += other.data[i];
    i++;
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
  std::size_t i = 0;
  for (auto& element : data) {
    element -= other;
    i++;
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

} // namespace measurement

} // namespace eduart