#include "sensorring/device/depth/tmf8829/TMF8829_ResultFormat.hpp"

#include "TMF8829_Constants.hpp"

namespace eduart {

namespace sensorring {

namespace device {

std::size_t TMF8829_ResultFormat::calculatePointSize() const {
  std::size_t size = 3; // Base size for distance

  if (signal_strength) {
    size += 2; // Add 2 bytes for signal strength
  }

  if (nr_of_peaks > 0) {
    size *= nr_of_peaks; // Multiply by the number of peaks if more than 0
  }

  if (xtalk) {
    size += 2; // Add 2 bytes for crosstalk
  }
  if (noise_strength) {
    size += 2; // Add 2 bytes for noise strength
  }

  return size;
}

} // namespace device

} // namespace sensorring

} // namespace eduart