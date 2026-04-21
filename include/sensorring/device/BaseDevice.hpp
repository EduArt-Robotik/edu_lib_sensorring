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

#include "sensorring/device/DeviceID.hpp"
#include "sensorring/device/DeviceState.hpp"
#include "sensorring/device/IDevice.hpp"
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
  math::Vector3 board_center_translation_offset;
  math::Vector3 board_center_rotation_offset;
};

/**
 * @class BaseDevice
 * @brief Single concrete base class for all devices in the SensorRing.
 *
 * Combines the polymorphic IDevice interface with communication, state tracking,
 * pose handling and promise-based measurement synchronisation.
 */
class SENSORRING_EXPORT BaseDevice : public virtual IDevice {
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

  // -- identity --

  DeviceID getDeviceID() const;
  unsigned int getIdx() const;

  // -- enable --

  bool getEnable() const;
  void setEnable(bool enable);

  // -- pose --

  void setPose(math::Vector3 translation, math::Vector3 rotation);
  void setPoseOffset(const DevicePoseOffset& offset) { _pose_offset = offset; }
  DevicePoseOffset getPoseOffset() const { return _pose_offset; }

  // -- state / measurement synchronisation --

  void resetSensorState();
  void clearDataFlag();
  std::future<bool> beginMeasurementWait();
  std::future<bool> beginDataAvailableWait();

protected:
  void setMeasurementReady(bool success);
  void setDataAvailableReady(bool success);

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

  std::optional<std::promise<bool> > _measurement_promise;
  std::optional<std::promise<bool> > _data_available_promise;
  std::mutex _promise_mutex;

  subscription::Subscription _com_subscription;
};

} // namespace device

} // namespace sensorring

} // namespace eduart