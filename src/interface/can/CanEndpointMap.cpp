#include "CanEndpointMap.hpp"

#include <limits>
#include <stdexcept>
#include <string>

#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace com {

CanEndpointMap* CanEndpointMap::getInstance() noexcept {
  static CanEndpointMap* instance = new CanEndpointMap;
  return instance;
}

CanEndpointMap::CanEndpointMap() {
  buildMap();
}

void CanEndpointMap::insertBidirectional(const std::string& endpoint, CanProtocol::canid id) {
  ComEndpoint ep(endpoint);

  _endpoint_to_id[ep] = id;

  auto it = _id_to_endpoint.find(id);
  if (it != _id_to_endpoint.end()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "CAN ID " + std::to_string(id) + " already mapped to \"" + it->second.getId() + "\", reverse lookup will not resolve to \"" + endpoint + "\"");
    return;
  }

  _id_to_endpoint.emplace(id, ep);
}

void CanEndpointMap::buildMap() {
  // Sensor board broadcast (ToF status broadcast)
  {
    CanProtocol::canid canid_tof_status, canid_tof_request, canid_broadcast;
    CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast);
    insertBidirectional("broadcast", canid_broadcast);
  }

  // ToF: shared status/request + per-index data
  {
    CanProtocol::canid canid_tof_status, canid_tof_request, canid_broadcast_status;
    CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast_status);
    insertBidirectional("tof_status", canid_tof_status);
    insertBidirectional("tof_request", canid_tof_request);

    CanProtocol::canid canid_tof_data_in, canid_tof_data_out, canid_broadcast;
    CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_DATA, canid_tof_data_in, canid_tof_data_out, canid_broadcast);
    for (std::size_t idx = 0; idx < MAX_SENSOR_BOARDS; ++idx) {
      if ((canid_tof_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "ToF index " + std::to_string(idx) + " would exceed CAN ID limit.");
        break;
      }
      insertBidirectional("tof" + std::to_string(idx) + "_data", static_cast<CanProtocol::canid>(canid_tof_data_in + idx));
    }
  }

  // Thermal: shared status/request + per-index data
  {
    CanProtocol::canid canid_thermal_status, canid_thermal_request, canid_thermal_broadcast_status;
    CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_STATUS, canid_thermal_status, canid_thermal_request, canid_thermal_broadcast_status);
    insertBidirectional("thermal_status", canid_thermal_status);
    insertBidirectional("thermal_request", canid_thermal_request);

    CanProtocol::canid canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast;
    CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_DATA, canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast);
    for (std::size_t idx = 0; idx < MAX_SENSOR_BOARDS; ++idx) {
      if ((canid_thermal_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Thermal index " + std::to_string(idx) + " would exceed CAN ID limit.");
        break;
      }
      insertBidirectional("thermal" + std::to_string(idx) + "_data", static_cast<CanProtocol::canid>(canid_thermal_data_in + idx));
    }
  }

  // Light (single endpoint)
  {
    CanProtocol::canid canid_light_in, canid_light_out, canid_light;
    CanProtocol::makeCanStdID(SYSID_LIGHT, NODEID_HEADLEFT, canid_light_in, canid_light_out, canid_light);
    insertBidirectional("light", canid_light);
  }
}

CanProtocol::canid CanEndpointMap::mapEndpointToId(ComEndpoint endpoint) const {
  return _endpoint_to_id.at(endpoint);
}

ComEndpoint CanEndpointMap::mapIdToEndpoint(CanProtocol::canid id) const {
  auto it = _id_to_endpoint.find(id);
  if (it != _id_to_endpoint.end()) {
    return it->second;
  }
  throw std::runtime_error("No Endpoint found for given CAN ID");
}

} // namespace com

} // namespace eduart
