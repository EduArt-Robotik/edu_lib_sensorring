#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "sensorring/interface/ComEndpoint.hpp"

#include "canprotocol.hpp"

namespace eduart {

namespace com {

/**
 * Singleton holding a bidirectional CAN endpoint <-> CAN ID map used by all CAN interface variants.
 * The map is built once on first access. Each entry must be unique in both directions;
 * collisions are detected at build time and silently skipped.
 */
class CanEndpointMap {
public:
  static constexpr std::size_t MAX_SENSOR_BOARDS = 256;

  static CanEndpointMap* getInstance() noexcept;

  CanProtocol::canid mapEndpointToId(ComEndpoint endpoint) const;
  ComEndpoint mapIdToEndpoint(CanProtocol::canid id) const;

private:
  CanEndpointMap();

  void buildMap();

  /**
   * Insert a bidirectional mapping. Skips the entry (with a warning) when the
   * CAN ID is already taken by another endpoint.
   */
  void insertBidirectional(const std::string& endpoint, CanProtocol::canid id);

  std::unordered_map<ComEndpoint, CanProtocol::canid> _endpoint_to_id;
  std::unordered_map<CanProtocol::canid, ComEndpoint> _id_to_endpoint;
};

} // namespace com

} // namespace eduart
