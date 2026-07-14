// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBoard.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstraction of a single sensor board
 * @date   2025-02-19
 */

#pragma once

#include <chrono>
#include <memory>
#include <mutex>

#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/Device.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/math/Pose.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

using namespace std::chrono_literals;

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
   * @brief Re-apply runtime configuration after a board reset.
   * @return true on success.
   */
  bool configure();

  /**
   * @brief Reset all boards on all interfaces (broadcast reset command).
   * @return true on success.
   */
  static bool resetBoards();

  /**
   * @brief Send enumeration command on the given interface so boards respond with CMD_ACTIVE_DEVICE_RESPONSE.
   * @param[in] interface Communication interface ID to enumerate.
   */
  static void cmdEnumerateBoards(com::ComInterfaceID interface);

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

  /**
   * @brief Set the board's orientation.
   * @param[in] orientation Orientation to set.
   * @return Return true if setting the orientation was successful.
   */
  bool setOrientation(Orientation orientation);

  /**
   * @brief Get the board's orientation.
   * @param[out] orientation Reference to store the current orientation.
   * @return Return true if reading the current orientation was successful.
   */
  bool getOrientation(Orientation& orientation);

  static constexpr std::chrono::milliseconds GET_PARAMETER_SLEEP   = 10ms;
  static constexpr std::chrono::milliseconds GET_PARAMETER_TIMEOUT = 100ms;

  unsigned int _idx;
  SensorBoardParams _params;

  board::EnumerationInformation _enum_info;
  std::vector<std::unique_ptr<device::Device> > _device_vec;

  std::atomic<bool> _got_update;
  com::ComInterface* _interface;
  mutable RecursiveMutex _com_mutex;
  subscription::Subscription _com_subscription;
};

} // namespace board

} // namespace sensorring

} // namespace eduart