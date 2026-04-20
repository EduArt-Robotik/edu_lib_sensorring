// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   BaseDevice.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base device combining IDevice and BaseSensor with device ID and state.
 * @date   2025-02-09
 */

#pragma once

#include "sensorring/math/Math.hpp"

#include "BaseSensor.hpp"
#include "DeviceID.hpp"
#include "DeviceState.hpp"
#include "IDevice.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/// Forward declaration of implementation of BaseDevice.
class SENSORRING_EXPORT DeviceImpl;

/**
 * @struct DevicePoseOffset
 * @brief Pose offset of a device relative to the center of its sensor board.
 */
struct DevicePoseOffset {

  /// Translation offset from the board center.
  math::Vector3 board_center_translation_offset;

  /// Rotation offset from the board center.
  math::Vector3 board_center_rotation_offset;
};

/**
 * @class BaseDevice
 * @brief Base class for concrete devices: implements IDevice and BaseSensor, holds device ID and state.
 */
class SENSORRING_EXPORT BaseDevice : public IDevice, public BaseSensor {
public:
  /**
   * @brief Constructs the device with the given ID, communication interface, endpoint, and enable flag.
   * @param[in] id Device identifier.
   * @param[in] interface Communication interface.
   * @param[in] target Communication endpoint.
   * @param[in] enable Whether the device is enabled.
   */
  BaseDevice(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable);

  /**
   * @brief Returns the device identifier.
   * @return DeviceID of this device.
   */
  DeviceID getDeviceID() const;

  /**
   * @brief Set the pose offset of this device relative to the board center.
   * @param[in] offset The pose offset to set.
   */
  void setPoseOffset(const DevicePoseOffset& offset) { _pose_offset = offset; }

  /**
   * @brief Get the pose offset of this device relative to the board center.
   * @return The current pose offset.
   */
  DevicePoseOffset getPoseOffset() const { return _pose_offset; }

  // void setEnable(bool enable);
  // bool getEnable() const;

protected:
  /// Current lifecycle/runtime state.
  DeviceState _state;

  /// Device identifier.
  DeviceID _id;

  /// Whether the device is enabled.
  bool _enable;

  /// Pose offset of the device relative to the sensor board center.
  DevicePoseOffset _pose_offset{
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 }
  };
};

} // namespace device

} // namespace sensorring

} // namespace eduart