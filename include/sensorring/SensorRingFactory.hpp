// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorRingFactory.hpp
 * @author EduArt Robotik GmbH
 * @brief  Factory for creating sensor rings via auto-discovery, configured expectations, or a mix of both.
 * @date   2026-02-19
 */

#pragma once

#include <unordered_map>
#include <variant>
#include <vector>

#include "sensorring/SensorBoard.hpp"
#include "sensorring/SensorBus.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Params.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace ring {

/**
 * @class SensorRingFactory
 * @brief Factory for creating a SensorRing.
 *
 * Usage:
 *  1. Call addInterface() for every communication bus to use.
 *  2. Optionally call expectBoard() (once per expected board, in index order) to
 *     declare what hardware should be present and which device params to apply.
 *  3. Optionally call setDefaultDeviceParams() to set params applied to every
 *     device of that type that does not have explicit params from expectBoard().
 *  4. Call build() to enumerate hardware, validate against expectations, and
 *     construct the SensorRing.
 *
 * If no expectBoard() calls are made for an interface, build() operates in pure
 * auto-discovery mode: every board found on the bus is used with default (or
 * setDefaultDeviceParams) configuration.
 */
class SENSORRING_EXPORT SensorRingFactory {
public:
  using DeviceParamsVariant = std::variant<device::VL53L8CX_Params, device::HTPA32_Params, device::WS2812b_Params>;

  /**
   * @brief Add a communication interface (bus) to scan during build().
   *
   * All subsequent expectBoard() calls apply to this interface until the next
   * addInterface() call.
   */
  void addInterface(com::ComInterfaceID interface);

  /**
   * @brief Declare an expected board on the current interface.
   *
   * Matched by index order against enumeration results. If params.board_type is
   * not Undefined it is validated against the hardware-reported type. All devices
   * reported by the hardware are instantiated; setDefaultDeviceParams() applies.
   */
  void expectBoard(device::SensorBoardParams params);

  /**
   * @brief Declare an expected board together with explicit per-device params.
   *
   * Only the device types present in @p device_params are instantiated; the
   * hardware must have at least those devices or build() fails. If
   * params.board_type is not Undefined it is additionally validated.
   */
  void expectBoard(device::SensorBoardParams params, std::vector<DeviceParamsVariant> device_params);

  /**
   * @brief Set default params applied to every device of the given type that has
   *        no explicit params from expectBoard().
   *
   * May be called multiple times for different device types.
   */
  void setDefaultDeviceParams(DeviceParamsVariant params);

  /**
   * @brief Enumerate hardware on all added interfaces, validate against
   *        expectations, and construct the SensorRing.
   *
   * @return Unique pointer to the SensorRing, or nullptr on failure.
   */
  std::unique_ptr<SensorRing> build();

  /**
   * @brief Reset the factory to its initial state.
   */
  void reset();

private:
  struct BoardExpectation {
    device::SensorBoardParams params;
    std::vector<DeviceParamsVariant> device_params;
    bool has_explicit_devices = false;
  };

  struct InterfaceConfig {
    com::ComInterfaceID interface;
    std::vector<BoardExpectation> expected_boards;
    bool has_expectations = false;
  };

  static device::DeviceType deviceTypeFromVariant(const DeviceParamsVariant& v);

  std::vector<InterfaceConfig> _interfaces;
  std::unordered_map<device::DeviceType, DeviceParamsVariant> _default_device_params;
};

} // namespace ring

} // namespace eduart