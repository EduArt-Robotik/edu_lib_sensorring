#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <vector>

#include "sensorring/device/Device.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"

#include "TMF8829_Measurement.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
} // namespace com

namespace device {

using namespace std::chrono_literals;

class TMF8829_Device;

/**
 * @class TMF8829_DeviceImpl
 * @brief Implementation class for TMF8829_Device hiding all private members.
 */
class TMF8829_DeviceImpl {
public:
  TMF8829_DeviceImpl(TMF8829_Device& parent, TMF8829_Params params, com::ComInterface* interface, unsigned int idx);
  ~TMF8829_DeviceImpl();

  const TMF8829_Params& getParams() const;

  bool setResolutionMode(ResolutionMode mode);
  bool getResolutionMode(ResolutionMode& mode);

  bool setIterationsSetting(std::uint16_t k_iterations);
  bool getIterationsSetting(std::uint16_t& k_iterations);

  bool setResultFormat(TMF8829_ResultFormat format);
  bool getResultFormat(TMF8829_ResultFormat& format);

  bool setResultFullNoise(bool full_noise);
  bool getResultFullNoise(bool& full_noise);

  bool setResultXtalk(bool xtalk);
  bool getResultXtalk(bool& xtalk);

  bool setResultNoiseStrength(bool noise_strength);
  bool getResultNoiseStrength(bool& noise_strength);

  bool setResultSignalStrength(bool signal_strength);
  bool getResultSignalStrength(bool& signal_strength);

  bool setResultNrOfPeaks(std::uint8_t nr_of_peaks);
  bool getResultNrOfPeaks(std::uint8_t& nr_of_peaks);

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

private:
  /**
   * @brief Validates whether the given parameter combination produces a result frame within the size limit.
   * @param params Proposed parameter set to validate.
   * @return True if valid, false otherwise (an error is logged).
   */
  bool isParamCombinationValid(const TMF8829_Params& params) const;

  static constexpr std::chrono::milliseconds GET_PARAMETER_SLEEP   = 10ms;
  static constexpr std::chrono::milliseconds GET_PARAMETER_TIMEOUT = 100ms;

  std::atomic<bool> _got_update;

  TMF8829_Params _params;
  TMF8829_Device& _parent;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
