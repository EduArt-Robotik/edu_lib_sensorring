// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Base class for all concrete sensor/actuator devices.
 * @date   2025-02-09
 */

#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <vector>

#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/action/ActionDispatcher.hpp"
#include "sensorring/device/types/DeviceID.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Pose.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace com {
class ComInterface;
}

namespace device {

/**
 * @class Device
 * @brief Base class for all devices in the SensorRing.
 *
 * Provides identity, communication, state tracking, pose handling and the
 * action queue used by the state machine.
 */
class SENSORRING_EXPORT Device : public ActionDispatcher {
public:
  /**
   * @brief Construct the device with identity and communication link.
   * @param[in] id        Device identifier.
   * @param[in] interface Communication interface.
   * @param[in] target    Communication endpoint this device listens to.
   */
  Device(DeviceID id, com::ComInterface* interface, com::ComEndpoint target);

  virtual ~Device();

  /**
   * @brief Return the full device identifier (type + index).
   */
  DeviceID getDeviceID() const;

  /**
   * @brief Return the zero-based hardware board index on its bus (used for CAN addressing and protocol bitmasks).
   */
  unsigned int getHwIdx() const;

  /**
   * @brief Reassign the logical device index (DeviceID.index) without changing the hardware index.
   * @param[in] index New globally unique per-type index.
   */
  void setDeviceIndex(unsigned int index);

  /**
   * @brief Apply post-reset runtime configuration for this device.
   * @return true on success.
   */
  virtual bool configure();

  /**
   * @brief Initialize board-level context pointer (non-owning).
   * @param[in] board_params Pointer to board params.
   */
  void setBoardContext(const board::SensorBoardParams* board_params);

  /**
   * @brief Return the board-level context pointer (non-owning).
   * @return Pointer to board params.
   */
  const board::SensorBoardParams* getBoardContext() const;

  /**
   * @brief Store a pose offset relative to the board centre.
   * @param[in] offset Offset to apply on top of the board centre pose.
   */
  void setPoseOffset(const math::Pose& offset);

  /**
   * @brief Return the stored pose offset relative to the board centre.
   * @return Pose offset relative to the board centre.
   */
  math::Pose getPoseOffset() const;

  /**
   * @brief Compute and return the global pose in the ring coordinate frame.
   * @return Global pose (board pose combined with device offset).
   */
  math::Pose getGlobalPose() const;

protected:
  virtual void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<std::uint8_t>& data) = 0;

  DeviceID _id;
  unsigned int _hw_idx;

  com::ComInterface* _interface;

  const board::SensorBoardParams* _board_params = nullptr;
  math::Pose _offset;

  subscription::Subscription _com_subscription;
};

} // namespace device

} // namespace sensorring

} // namespace eduart