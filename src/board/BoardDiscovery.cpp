#include "sensorring/board/BoardDiscovery.hpp"

#include <chrono>
#include <sensorring_transport/Protocol.hpp>
#include <sstream>
#include <thread>

#include "board/BoardEnumerator.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {
namespace sensorring {
namespace board {

using namespace eduart::sensorring::transport::protocol;

void BoardDiscovery::addInterface(com::SocketCanParams params) {
  _interfaces.push_back(std::make_unique<com::SocketCanParams>(std::move(params)));
}

void BoardDiscovery::addInterface(com::UsbTingoParams params) {
  _interfaces.push_back(std::make_unique<com::UsbTingoParams>(std::move(params)));
}

BoardDiscovery::EnumerationMap BoardDiscovery::enumerate() {
  EnumerationMap result;
  for (const auto& params : _interfaces) {
    try {
      com::ComInterface* iface = nullptr;
      if (const auto* socketcan = dynamic_cast<const com::SocketCanParams*>(params.get())) {
        iface = com::ComManager::getInstance()->getInterface(*socketcan);
      } else if (const auto* usbtingo = dynamic_cast<const com::UsbTingoParams*>(params.get())) {
        iface = com::ComManager::getInstance()->getInterface(*usbtingo);
      }
      if (!iface) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Could not open interface " + params->name + " - skipping.");
        continue;
      }
      auto id    = iface->getID();
      result[id] = queryConnectedDevices(id);
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Error while enumerating interface " + params->name + " - skipping: " + e.what());
    }
  }
  _enumeration_results = result;
  return result;
}

const BoardDiscovery::EnumerationMap& BoardDiscovery::getLatestEnumerationResult() const {
  return _enumeration_results;
}

std::string BoardDiscovery::printTopology() const {
  return printTopology(_enumeration_results);
}

std::vector<EnumerationInformation> BoardDiscovery::queryConnectedDevices(com::ComInterfaceID interface, unsigned int timeout_ms) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (!iface) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unable to open interface " + interface.name + " for enumeration.");
    return {};
  }

  BoardEnumerator enumerator(iface);
  enumerator.startEnumeration();
  std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
  return enumerator.getResult();
}

void BoardDiscovery::cmdEnumerateBoards(com::ComInterfaceID interface) {
  auto* iface = com::ComManager::getInstance()->getInterface(interface);
  if (!iface) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unable to open interface " + interface.name + " for enumeration.");
    return;
  }
  BoardEnumerator::sendEnumerationRequest(*iface);
}

bool BoardDiscovery::resetBoards() {
  bool success = true;
  for (auto* iface : com::ComManager::getInstance()->getInterfaces()) {
    if (!iface->send(com::ComEndpoint{ com::Direction::Broadcast, com::ComEndpoint::BROADCAST, devbyte::BOARD }, sensor_board::RESET, {})) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Failed to reset boards on interface " + iface->getID().name + ".");
      success = false;
    }
  }
  return success;
}

std::string BoardDiscovery::printTopology(const EnumerationMap& results) {
  std::stringstream ss;
  for (const auto& [iface_id, boards] : results) {
    ss << std::endl << std::endl;
    ss << "=================================================" << std::endl;
    ss << "Topology of the sensors on " << iface_id.name << ":" << std::endl;
    ss << std::endl;
    for (const auto& board : boards) {
      ss << board.toString() << std::endl;
    }
    ss << "=================================================" << std::endl;
  }
  return ss.str();
}

} // namespace board
} // namespace sensorring
} // namespace eduart
