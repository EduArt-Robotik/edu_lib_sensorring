// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   BaseDevice.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base class for all concrete sensor/actuator devices.
 * @date   2025-02-09
 */

#pragma once

#include <cstdint>
#include <future>
#include <mutex>
#include <optional>
#include <vector>

#include "sensorring/device/types/DeviceID.hpp"
#include "sensorring/device/types/DeviceState.hpp"
// #include "sensorring/device/IDevice.hpp" // removed, merged into BaseDevice
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/math/Matrix3.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

namespace device {

/**
 * @struct DevicePoseOffset
 * @brief Pose offset of a device relative to the center of its sensor board.
 */
struct DevicePoseOffset {
  math::Vector3 board_center_translation_offset; ///< Translation (x, y, z in metres) from the board centre to the device.
  math::Vector3 board_center_rotation_offset;    ///< Rotation offset (roll, pitch, yaw in degrees) from the board centre to the device.
};

/**
 * @class BaseDevice
 * @brief Single concrete base class for all devices in the SensorRing.
 *
 * Combines the polymorphic IDevice interface with communication, state tracking,
 * pose handling and promise-based measurement synchronisation.
 */
class SENSORRING_EXPORT BaseDevice {
public:
  /**
   * @brief Construct the device with identity, communication link and enable flag.
   * @param[in] id        Device identifier.
   * @param[in] interface Communication interface.
   * @param[in] target    Communication endpoint this device listens to.
   * @param[in] enable    Whether the device starts enabled.
   */
  BaseDevice(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable);

  virtual ~BaseDevice();

  /**
   * @brief Enqueue a self-contained action to be executed by the state machine.
   * @param[in] action Callable executed once during the next device_actions slot.
   *                   Should be non-blocking and exception-safe.
   */
  void enqueueAction(std::function<void()> action);

  /**
   * @brief Atomically drain and return all pending actions.
   * @return Vector of actions to execute. Empty if no actions were pending.
   */
  std::vector<std::function<void()> > drainActions();

  // -- identity --

  /// @brief Return the full device identifier (type + index).
  DeviceID getDeviceID() const;

  /// @brief Return the zero-based index of this device on its bus.
  unsigned int getIdx() const;

  // -- enable --

  /// @brief Return whether this device is currently enabled.
  bool getEnable() const;

  /**
   * @brief Apply post-reset runtime configuration for this device.
   * @return true on success.
   */
  virtual bool configure();

  /**
   * @brief Enable or disable this device.
   * @param[in] enable @c true to enable, @c false to disable.
   */
  void setEnable(bool enable);

  // -- pose --

  /**
   * @brief Set the absolute pose of this device in the ring coordinate frame.
   * @param[in] translation Position (x, y, z) in metres.
   * @param[in] rotation    Orientation (roll, pitch, yaw) in degrees.
   */
  void setPose(math::Vector3 translation, math::Vector3 rotation);

  /**
   * @brief Store a pose offset relative to the board centre.
   * @param[in] offset Offset to apply on top of the board centre pose.
   */
  void setPoseOffset(const DevicePoseOffset& offset) { _pose_offset = offset; }

  /// @brief Return the stored pose offset relative to the board centre.
  DevicePoseOffset getPoseOffset() const { return _pose_offset; }

  // -- state / measurement synchronisation --

  /**
   * @brief Begin an asynchronous wait for the next measurement trigger.
   * @return Future that resolves to @c true when the measurement trigger fires, or @c false on shutdown.
   */
  std::future<bool> beginMeasurementWait();

  /**
   * @brief Satisfy the pending measurement-wait future.
   * @param[in] success @c true if the measurement succeeded, @c false on error.
   */
  void setMeasurementReady(bool success);

  /**
   * @brief Begin an asynchronous wait for new data to become available.
   * @return Future that resolves to @c true when data is available, or @c false on shutdown.
   */
  std::future<bool> beginDataAvailableWait();

  /**
   * @brief Satisfy the pending data-available-wait future.
   * @param[in] success @c true if data is ready, @c false on error.
   */
  void setDataAvailableReady(bool success);

  /// @brief Reset the device to its initialised state, clearing all error flags.
  void resetSensorState();

  /// @brief Clear the data-available flag so the device can accept the next measurement cycle.
  void clearDataFlag();

protected:
  std::mutex _action_mutex;
  std::vector<std::function<void()> > _pending_actions;
  virtual void onResetSensorState() {}
  virtual void onClearDataFlag() {}

  virtual void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<std::uint8_t>& data) = 0;

  // -- members --

  DeviceID _id;
  unsigned int _idx;
  DeviceState _state;
  bool _enable;

  com::ComInterface* _interface;

  math::Vector3 _translation;
  math::Vector3 _rotation;
  math::Matrix3 _rot_m;

  DevicePoseOffset _pose_offset{
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 }
  };

  mutable std::mutex _state_mutex;

  std::mutex _promise_mutex;
  std::optional<std::promise<bool> > _data_available_promise;
  std::optional<std::promise<bool> > _measurement_promise;

  subscription::Subscription _com_subscription;
};

} // namespace device

} // namespace sensorring

} // namespace eduart