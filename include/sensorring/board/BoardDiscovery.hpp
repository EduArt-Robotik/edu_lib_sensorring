// Copyright (c) 2026 EduArt Robotik GmbH

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/interface/InterfaceParams.hpp"
#include "sensorring/platform/SensorringBaseExport.hpp"

namespace eduart {
namespace sensorring {
namespace board {

/**
 * @brief Discovers boards without constructing or configuring sensor devices.
 *
 * Interface probing tolerates unavailable interfaces and logs a warning before
 * continuing. Single-interface queries propagate communication exceptions.
 */
class SENSORRING_BASE_EXPORT BoardDiscovery {
public:
  using EnumerationMap = std::unordered_map<com::ComInterfaceID, std::vector<EnumerationInformation> >;

  void addInterface(com::SocketCanParams params);
  void addInterface(com::UsbTingoParams params);

  EnumerationMap enumerate();
  const EnumerationMap& getLatestEnumerationResult() const;
  std::string printTopology() const;

  /// Query boards, waiting timeout_ms milliseconds for enumeration responses.
  static std::vector<EnumerationInformation> queryConnectedDevices(com::ComInterfaceID interface, unsigned int timeout_ms = 250);
  /// Send the enumeration request on one interface.
  static void cmdEnumerateBoards(com::ComInterfaceID interface);
  /// Reset boards on all interfaces already opened by the communication manager.
  static bool resetBoards();
  static std::string printTopology(const EnumerationMap& results);

private:
  std::vector<std::unique_ptr<com::InterfaceParams> > _interfaces;
  EnumerationMap _enumeration_results;
};

} // namespace board
} // namespace sensorring
} // namespace eduart
