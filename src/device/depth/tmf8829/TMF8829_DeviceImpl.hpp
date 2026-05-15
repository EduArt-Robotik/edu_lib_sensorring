#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "sensorring/device/BaseDevice.hpp"
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

  bool getResolutionMode(ResolutionMode& mode);

  bool setResolutionMode(ResolutionMode mode);

  std::pair<const measurement::DepthMeasurement&, DeviceState> getLatestMeasurement() const;

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

private:
  static constexpr unsigned int RESOLUTION                         = 64;
  static constexpr std::chrono::milliseconds GET_PARAMETER_TIMEOUT = 100ms;

  int _resolution_mode;

  TMF8829_Device& _parent;
  const TMF8829_Params _params;
  TMF8829_Measurement _latest_measurement;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
