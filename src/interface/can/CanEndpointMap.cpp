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

void CanEndpointMap::addSensorBoardEndpoint() {
  CanProtocol::canid canid_tof_status, canid_tof_request, canid_broadcast;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast);

  _id_map[ComEndpoint("broadcast")] = canid_broadcast;
}

void CanEndpointMap::addTofSensorEndpoint(std::size_t idx) {
  CanProtocol::canid canid_tof_data_in, canid_tof_data_out, canid_broadcast;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_DATA, canid_tof_data_in, canid_tof_data_out, canid_broadcast);

  if ((canid_tof_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Sensor index +" + std::to_string(idx) + " results in a CAN address that is outside the numeric limits of CAN addresses.");
  }

  _id_map[ComEndpoint("tof" + std::to_string(idx) + "_data")] = static_cast<CanProtocol::canid>(canid_tof_data_in + idx);

  CanProtocol::canid canid_tof_status, canid_tof_request, canid_broadcast_status;
  CanProtocol::makeCanStdID(SYSID_TOF, NODEID_TOF_STATUS, canid_tof_status, canid_tof_request, canid_broadcast_status);

  _id_map[ComEndpoint("tof_status")]  = canid_tof_status;
  _id_map[ComEndpoint("tof_request")] = canid_tof_request;
}

void CanEndpointMap::addThermalSensorEndpoint(std::size_t idx) {
  CanProtocol::canid canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast;
  CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_DATA, canid_thermal_data_in, canid_thermal_data_out, canid_thermal_broadcast);

  if ((canid_thermal_data_in + idx) > std::numeric_limits<CanProtocol::canid>::max()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Sensor index +" + std::to_string(idx) + " results in a CAN address that is outside the numeric limits of CAN addresses.");
  }

  _id_map[ComEndpoint("thermal" + std::to_string(idx) + "_data")] = static_cast<CanProtocol::canid>(canid_thermal_data_in + idx);

  CanProtocol::canid canid_thermal_status, canid_thermal_request, canid_thermal_broadcast_status;
  CanProtocol::makeCanStdID(SYSID_THERMAL, NODEID_THERMAL_STATUS, canid_thermal_status, canid_thermal_request, canid_thermal_broadcast_status);

  _id_map[ComEndpoint("thermal_status")]  = canid_thermal_status;
  _id_map[ComEndpoint("thermal_request")] = canid_thermal_request;
}

void CanEndpointMap::addLightSensorEndpoint() {
  CanProtocol::canid canid_light_in, canid_light_out, canid_light;
  CanProtocol::makeCanStdID(SYSID_LIGHT, NODEID_HEADLEFT, canid_light_in, canid_light_out, canid_light);

  _id_map[ComEndpoint("light")] = canid_light;
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
