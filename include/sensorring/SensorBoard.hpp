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

#include "sensorring/SensorBoardParams.hpp"
#include "sensorring/device/BaseDevice.hpp"
#include "sensorring/device/EnumerationInformation.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

namespace device {

/**
 * @class SensorBoard
 * @brief One sensor board on a bus: holds configured devices and receives COM callbacks for enumeration and data.
 */
class SENSORRING_EXPORT SensorBoard : public IDevice {
public:
  /**
   * @brief Construct the board with parameters, communication interface, index, and owned devices.
   * @param[in] params Board configuration (pose, device params).
   * @param[in] interface Communication interface ID for this board.
   * @param[in] idx Board index on the bus.
   * @param[in] devices Owned devices (sensors/actuators) on this board.
   */
  SensorBoard(SensorBoardParams params, com::ComInterfaceID interface, unsigned int idx, std::vector<std::unique_ptr<BaseDevice> > devices);
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
  const EnumerationInformation& getEnumInfo() const;
  /**
   * @brief Board type from configuration (undefined if not set). Used for topology enforcement.
   * @return Board type.
   */
  SensorBoardType getBoardType() const;

  /**
   * @brief Return non-owning pointers to all devices on this board.
   * @return Vector of BaseDevice pointers.
   */
  std::vector<BaseDevice*> getDevices() const;

private:
  /**
   * @brief Handle incoming COM message; used for enumeration and device data.
   * @param[in] source Endpoint that received the message.
   * @param[in] data Raw message payload.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

  unsigned int _idx;
  com::ComInterface* _interface;
  const SensorBoardParams _params;
  EnumerationInformation _enum_info;

  std::vector<std::unique_ptr<device::BaseDevice> > _device_vec;

  mutable std::recursive_mutex _com_mutex;
  using LockGuard = std::lock_guard<std::recursive_mutex>;

  subscription::Subscription _com_subscription;
};

} // namespace device

} // namespace sensorring

} // namespace eduart