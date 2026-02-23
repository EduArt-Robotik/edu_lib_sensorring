#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "sensorring/interface/ComEndpoint.hpp"

#include "canprotocol.hpp"

namespace eduart {

namespace com {

/**
 * Singleton holding the single shared CAN endpoint <-> CAN ID map to be used by all CAN interface variants.
 * The map is built once with a reserved range of device IDs (e.g. 256 sensor boards); no dynamic
 * registration is required, avoiding initialization-order issues.
 */
class CanEndpointMap {
public:
  /** Number of device indices reserved (tof0..tof(N-1), thermal0..thermal(N-1)). */
  static constexpr std::size_t MAX_SENSOR_BOARDS = 256;

  /**
   * Get a pointer to the instance of the CanEndpointMap singleton.
   * The map is fully populated on first access.
   * @return Pointer to the CanEndpointMap instance
   */
  static CanEndpointMap* getInstance() noexcept;

  CanProtocol::canid mapEndpointToId(ComEndpoint endpoint) const;
  ComEndpoint mapIdToEndpoint(CanProtocol::canid id) const;

private:
  CanEndpointMap();

  void buildMap();

  std::unordered_map<ComEndpoint, CanProtocol::canid> _id_map;
};

} // namespace com

} // namespace eduart
