// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   BaseDevice.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base device combining IDevice and BaseSensor with device ID and state.
 * @date   2025-02-09
 */

#pragma once

#include "BaseSensor.hpp"
#include "DeviceID.hpp"
#include "IDevice.hpp"

namespace eduart {

namespace device {

class SENSORRING_EXPORT DeviceImpl;

/**
 * @struct DeviceParams
 * @brief Parameters for device creation: identifier and enable flag.
 */
struct SENSORRING_EXPORT DeviceParams {
  /// Device identifier (type, name, index).
  DeviceID id;
  /// Whether the device is enabled at creation.
  bool enabled;
};

/**
 * @enum DeviceState
 * @brief Lifecycle and runtime state of a device.
 */
enum class SENSORRING_EXPORT DeviceState {
  UNDEFINED,
  INITIALIZED,
  IDLE,
  ERROR,
  SHUTDOWN
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
  /// Destructor
  virtual ~BaseDevice() = default;

  /**
   * @brief Returns the device identifier.
   * @return DeviceID of this device.
   */
  DeviceID getDeviceID() const;

  //void setEnable(bool enable);
  //bool getEnable() const;

protected:
  /// Current lifecycle/runtime state.
  DeviceState _state;
  /// Device identifier.
  DeviceID _id;
  /// Whether the device is enabled.
  bool _enable;
};

} // namespace device

} // namespace eduart