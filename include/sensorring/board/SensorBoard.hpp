// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoard.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstraction of a single sensor board
 * @date   2025-02-19
 */

#pragma once

#include <memory>
#include <mutex>

#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/Device.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/math/Pose.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

namespace board {

/**
 * @class SensorBoard
 * @brief One sensor board on a bus: holds configured devices and receives COM callbacks for enumeration and data.
 */
class SENSORRING_EXPORT SensorBoard {
public:
  /**
   * @brief Construct the board with parameters, communication interface, index, and owned devices.
   * @param[in] params Board configuration (pose, device params).
   * @param[in] interface Communication interface ID for this board.
   * @param[in] idx Board index on the bus.
   * @param[in] devices Owned devices (sensors/actuators) on this board.
   */
  SensorBoard(SensorBoardParams params, com::ComInterfaceID interface, unsigned int idx, std::vector<std::unique_ptr<device::Device> > devices);
  /// Destructor
  ~SensorBoard();

  /**
   * @brief Report whether this board has completed enumeration.
   * @return true if enumeration response has been received.
   */
  bool isEnumerated() const;
  /**
   * @brief Return the enumeration info received from the hardware.
   * @return Const reference to enumeration information.
   */
  const board::EnumerationInformation& getEnumInfo() const;
  /**
   * @brief Board type from configuration (undefined if not set). Used for topology enforcement.
   * @return Board type.
   */
  SensorBoardType getBoardType() const;

  /**
   * @brief Return non-owning pointers to all devices on this board.
   * @return Vector of Device pointers.
   */
  std::vector<device::Device*> getDevices() const;

  /**
   * @brief Return the board's pose in the ring coordinate frame.
   * @return Const reference to the board pose.
   */
  const math::Pose& getPose() const;

private:
  using Mutex      = std::mutex;
  using UniqueLock = std::unique_lock<Mutex>;

  using RecursiveMutex = std::recursive_mutex;
  using RecursiveLock  = std::lock_guard<RecursiveMutex>;

  /**
   * @brief Handle incoming COM message; used for enumeration and device data.
   * @param[in] source Endpoint that received the message.
   * @param[in] data Raw message payload.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

  unsigned int _idx;
  com::ComInterface* _interface;
  const SensorBoardParams _params;
  math::Pose _pose;
  board::EnumerationInformation _enum_info;

  mutable RecursiveMutex _com_mutex;

  std::vector<std::unique_ptr<device::Device> > _device_vec;

  subscription::Subscription _com_subscription;
};

} // namespace board

} // namespace sensorring

} // namespace eduart