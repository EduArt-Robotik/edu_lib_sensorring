#include "CanEndpointMap.hpp"

#include <algorithm>
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

void CanEndpointMap::buildMap() {
  // Sensor board broadcast (ToF status broadcast)
  {
    CanProtocol::canid canid_tof_status, canid_tof_request, canid_broadcast;
    CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast);
    _id_map[ComEndpoint("broadcast")] = canid_broadcast;
  }

  // ToF: shared status/request + per-index data
  {
    CanProtocol::canid canid_tof_status, canid_tof_request, canid_broadcast_status;
    CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast_status);
    _id_map[ComEndpoint("tof_status")]  = canid_tof_status;
    _id_map[ComEndpoint("tof_request")] = canid_tof_request;

    CanProtocol::canid canid_tof_data_in, canid_tof_data_out, canid_broadcast;
    CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_DATA, canid_tof_data_in, canid_tof_data_out, canid_broadcast);
    for (std::size_t idx = 0; idx < MAX_SENSOR_BOARDS; ++idx) {
      if ((canid_tof_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Exception,
            "ToF index " + std::to_string(idx) + " would exceed CAN ID limit.");
        break;
      }
      _id_map[ComEndpoint("tof" + std::to_string(idx) + "_data")] =
          static_cast<CanProtocol::canid>(canid_tof_data_in + idx);
    }
  }

  // Thermal: shared status/request + per-index data
  {
    CanProtocol::canid canid_thermal_status, canid_thermal_request, canid_thermal_broadcast_status;
    CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_STATUS, canid_thermal_status, canid_thermal_request, canid_thermal_broadcast_status);
    _id_map[ComEndpoint("thermal_status")]  = canid_thermal_status;
    _id_map[ComEndpoint("thermal_request")] = canid_thermal_request;

    CanProtocol::canid canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast;
    CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_DATA, canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast);
    for (std::size_t idx = 0; idx < MAX_SENSOR_BOARDS; ++idx) {
      if ((canid_thermal_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Exception,
            "Thermal index " + std::to_string(idx) + " would exceed CAN ID limit.");
        break;
      }
      _id_map[ComEndpoint("thermal" + std::to_string(idx) + "_data")] =
          static_cast<CanProtocol::canid>(canid_thermal_data_in + idx);
    }
  }

  // Light (single endpoint)
  {
    CanProtocol::canid canid_light_in, canid_light_out, canid_light;
    CanProtocol::makeCanStdID(SYSID_LIGHT, NODEID_HEADLEFT, canid_light_in, canid_light_out, canid_light);
    _id_map[ComEndpoint("light")] = canid_light;
  }
}

CanProtocol::canid CanEndpointMap::mapEndpointToId(ComEndpoint endpoint) const {
  return _id_map.at(endpoint);
}

ComEndpoint CanEndpointMap::mapIdToEndpoint(CanProtocol::canid id) const {
  auto it = std::find_if(_id_map.begin(), _id_map.end(), [&id](const auto& pair) {
    return pair.second == id;
  });

  if (it != _id_map.end()) {
    return it->first;
  }
  throw std::runtime_error("No Endpoint found for given CAN ID");
}

} // namespace com

} // namespace eduart
