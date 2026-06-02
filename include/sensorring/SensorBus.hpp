// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBus.hpp
 * @author EduArt Robotik GmbH
 * @brief  One communication bus owning multiple sensor boards.
 * @date   2025-02-19
 */

#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "sensorring/board/SensorBoard.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

using namespace std::chrono_literals;

/**
 * @class SensorBus
 * @brief One communication interface (e.g. CAN) owning multiple SensorBoards and forwarding COM messages.
 */
class SENSORRING_EXPORT SensorBus {
public:
  /**
   * @brief Construct the bus with a communication interface and owned sensor boards.
   * @param[in] interface Communication interface ID for this bus.
   * @param[in] board_vec Owned sensor boards.
   */
  SensorBus(com::ComInterfaceID interface, std::vector<std::unique_ptr<board::SensorBoard> > board_vec);

  /**
   * @brief Enable or disable bit rate switching on the bus interface.
   * @param[in] brs_enable Enable flag.
   */
  void setBitRateSwitching(bool brs_enable);

  /**
   * @brief Total number of sensor boards on this bus.
   * @return Number of boards.
   */
  unsigned int getSensorCount() const;

  /**
   * @brief Communication interface used by this bus.
   * @return Pointer to the ComInterface.
   */
  com::ComInterface* getInterface() const;

  /**
   * @brief Non-owning pointers to all sensor boards on this bus.
   * @return Vector of SensorBoard pointers.
   */
  std::vector<board::SensorBoard*> getSensorBoards() const;

  /**
   * @brief Enumerate boards on an interface.
   * @param[in] interface Communication interface ID to enumerate.
   * @param[in] timeout Time to wait for responses before returning.
   * @return Vector of enumeration info. May be empty if none or on error.
   */
  static std::vector<board::EnumerationInformation> queryConnectedDevices(com::ComInterfaceID interface, std::chrono::milliseconds timeout = 250ms);

private:
  com::ComInterface* _interface;

  std::vector<std::unique_ptr<board::SensorBoard> > _board_vec;

  subscription::Subscription _com_subscription;
};

} // namespace sensorring

} // namespace eduart