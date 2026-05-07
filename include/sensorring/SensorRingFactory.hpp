// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorRingFactory.hpp
 * @author EduArt Robotik GmbH
 * @brief  Factory for creating sensor rings via auto-discovery, configured expectations, or a mix of both.
 * @date   2026-02-19
 */

#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "sensorring/SensorRing.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
#include "sensorring/device/types/EnumerationInformation.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace ring {

/**
 * @enum ValidationMode
 * @brief Controls how build() handles mismatches between expectations and discovered hardware.
 */
enum class ValidationMode {
  /// All expectations must match exactly by index; build() returns nullptr on any mismatch.
  Strict,
  /// Expectations are matched by searching for compatible boards (board type and
  /// required devices) rather than by index. Unmatched expectations are logged
  /// as warnings; build() succeeds with whatever subset could be reconciled.
  Relaxed
};

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
 *
 * When expectations are present, the validation mode controls how mismatches are
 * handled:
 *  - Strict (default): boards are matched by index; any mismatch fails the build.
 *  - Relaxed: for each expectation, the factory searches all unclaimed boards
 *    for the first one with a compatible board type and the required device
 *    types. Boards that are not claimed by any expectation remain unconfigured.
 */
class SENSORRING_EXPORT SensorRingFactory {
public:
  /// Currently supported devices for explicit configuration in expectBoard().
  using DeviceParamsVariant = std::variant<device::VL53L8CX_Params, device::HTPA32_Params, device::WS2812b_Params>;

  /// Per-interface enumeration results, keyed by interface ID.
  using EnumerationMap = std::unordered_map<com::ComInterfaceID, std::vector<device::EnumerationInformation> >;

  /// Minimum sensor board firmware version required by this library version.
  static constexpr Version MIN_FIRMWARE_VERSION = { 0, 9, 0 };

  /**
   * @brief Construct the factory with a validation mode.
   * @param[in] mode Validation mode (default: Relaxed). In Relaxed mode,
   *            mismatched boards are skipped instead of aborting.
   */
  explicit SensorRingFactory(ValidationMode mode = ValidationMode::Relaxed);

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
   * Uses the ValidationMode set in the constructor.
   * @return Unique pointer to the SensorRing, or nullptr on failure.
   */
  std::unique_ptr<SensorRing> build();

  /**
   * @brief Enumerate hardware on all added interfaces without building a SensorRing.
   *
   * Useful for interactive applications that want to discover connected boards
   * before committing to a build. Results are stored and retrievable via
   * getLatestEnumerationResult(). Also called internally by build().
   *
   * @return Per-interface enumeration results.
   */
  EnumerationMap enumerate();

  /**
   * @brief Return the enumeration results from the last enumerate() or build() call.
   * @return Per-interface enumeration results. Empty if neither method has been called yet.
   */
  const EnumerationMap& getLatestEnumerationResult() const;

  /**
   * @brief Format the latest enumeration results as a human-readable topology string.
   * @return Formatted topology string. Empty if no enumeration has been performed.
   */
  std::string printTopology() const;

  /**
   * @brief Reset the factory to its initial state. Enumeration results are preserved.
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

  /// Build a DeviceParamsMap for the given device types, applying user defaults where available.
  std::unordered_map<device::DeviceType, DeviceParamsVariant> buildDefaultParamsMap(const std::vector<device::DeviceType>& devices) const;

  std::vector<InterfaceConfig> _interfaces;
  std::unordered_map<device::DeviceType, DeviceParamsVariant> _default_device_params;
  EnumerationMap _enumeration_results;
  ValidationMode _mode;
};

} // namespace ring

} // namespace sensorring

} // namespace eduart