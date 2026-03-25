// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBus.hpp
 * @author EduArt Robotik GmbH
 * @brief  One communication bus owning multiple sensor boards.
 * @date   2025-02-19
 */

#pragma once

#include <memory>
#include <vector>

#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/interface/ComObserver.hpp"

#include "SensorBoard.hpp"

namespace eduart {

namespace com {
class ComInterface;
}

namespace bus {

using namespace std::chrono_literals;

/**
 * @class SensorBus
 * @brief One communication interface (e.g. CAN) owning multiple SensorBoards and forwarding COM messages.
 */
class SensorBus : public com::ComObserver {
public:
  /**
   * @brief Construct the bus with a communication interface and owned sensor boards.
   * @param[in] interface Communication interface ID for this bus.
   * @param[in] board_vec Owned sensor boards.
   */
  SensorBus(com::ComInterfaceID interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec);

  /// Destructor
  ~SensorBus();

  /**
   * @brief Enable or disable bit rate switching on the bus interface.
   * @param[in] brs_enable Enable flag.
   */
  void setBrs(bool brs_enable);

  /**
   * @brief Total number of sensor boards on this bus.
   * @return Number of boards.
   */
  size_t getSensorCount() const;

  /**
   * @brief Communication interface used by this bus.
   * @return Pointer to the ComInterface.
   */
  com::ComInterface* getInterface() const;

  /**
   * @brief Non-owning pointers to all sensor boards on this bus.
   * @return Vector of SensorBoard pointers.
   */
  std::vector<device::SensorBoard*> getSensorBoards() const;

  /**
   * @brief Enumerate boards on an interface.
   * @param[in] interface Communication interface ID to enumerate.
   * @return Vector of enumeration info. May be empty if none or on error.
   */
  static std::vector<device::EnumerationInformation> queryConnectedDevices(com::ComInterfaceID interface);

private:
  /**
   * @brief Handle incoming COM message; used for enumeration counts and forwarding to boards.
   * @param[in] source Endpoint that received the message.
   * @param[in] data Raw message payload.
   */
  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

  static constexpr std::chrono::milliseconds ENUMERATION_TIMEOUT = 250ms;

  com::ComInterface* _interface;

  std::vector<std::unique_ptr<device::SensorBoard> > _board_vec;
};

} // namespace bus

} // namespace eduart