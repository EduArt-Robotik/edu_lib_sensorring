#pragma once

#include <cstdint>
#include <unordered_map>

#include "interface/ComEndpoints.hpp"

#include "canprotocol.hpp"

namespace eduart {

namespace com {

/**
 * Singleton holding the single shared CAN endpoint <-> CAN ID map to be used by all CAN interface variants.
 */
class CanEndpointMap {
public:
  /**
   * Get a pointer to the instance of the CanEndpointMap singleton.
   * @return Pointer to the CanEndpointMap instance
   */
  static CanEndpointMap* getInstance() noexcept;

  void addSensorBoardEndpoint();
  void addTofSensorEndpoint(std::size_t idx);
  void addThermalSensorEndpoint(std::size_t idx);
  void addLightSensorEndpoint();

  CanProtocol::canid mapEndpointToId(ComEndpoint endpoint) const;
  ComEndpoint mapIdToEndpoint(CanProtocol::canid id) const;

private:
  CanEndpointMap() = default;

  std::unordered_map<ComEndpoint, CanProtocol::canid> _id_map;
};

} // namespace com

} // namespace eduart
