// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorRingFactory.hpp
 * @author EduArt Robotik GmbH
 * @brief  Factory for creating sensor rings via auto-discovery, configured expectations, or a mix of both.
 * @date   2026-02-19
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "sensorring/SensorRing.hpp"
#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/AnyDeviceParams.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/interface/InterfaceParams.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

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
 *  - Strict: boards are matched by index; any mismatch fails the build.
 *  - Relaxed (default): for each expectation, the factory searches all unclaimed boards
 *    for the first one with a compatible board type and the required device
 *    types. Boards that are not claimed by any expectation remain unconfigured.
 */
class SENSORRING_EXPORT SensorRingFactory {
public:
  /// Per-interface enumeration results, keyed by interface ID.
  using EnumerationMap = std::unordered_map<com::ComInterfaceID, std::vector<board::EnumerationInformation> >;

  /// Minimum sensor board firmware version required by this library version.
  static constexpr Version MIN_FIRMWARE_VERSION = { 0, 9, 0 };

  /**
   * @brief Construct the factory with a validation mode.
   * @param[in] mode Validation mode (default: Relaxed). In Relaxed mode,
   *            mismatched boards are skipped instead of aborting.
   */
  explicit SensorRingFactory(ValidationMode mode = ValidationMode::Relaxed);

  // ── Interface configuration ──

  /**
   * @brief Add a SocketCAN interface to scan during build().
   *
   * All subsequent expectBoard() calls apply to this interface until the next
   * addInterface() call.
   * @param[in] params SocketCAN configuration parameters.
   */
  void addInterface(com::SocketCanParams params);

  /**
   * @brief Add a USBtingo interface to scan during build().
   *
   * All subsequent expectBoard() calls apply to this interface until the next
   * addInterface() call.
   * @param[in] params USBtingo configuration parameters.
   */
  void addInterface(com::UsbTingoParams params);

  // ── Board expectations ──

  /**
   * @brief Declare an expected board on the current interface.
   *
   * Boards are matched against enumeration results using the configured
   * ValidationMode. Use expectDevice() after this call to declare which
   * devices should be instantiated on this board.
   *
   * If no expectDevice() calls follow, all devices reported by the hardware
   * are instantiated with default (or setDefaultDeviceParams()) configuration.
   *
   * @param[in] params Board-level parameters (type, name, pose, etc.).
   */
  void expectBoard(board::SensorBoardParams params);

  // ── Device expectations (applied to the last expectBoard) ──

  /**
   * @brief Declare an expected device on the most recently added board.
   *
   * Only the device types declared via expectDevice() are instantiated for
   * that board. The provided params override any defaults set via
   * setDefaultDeviceParams().
   *
   * Must be called after expectBoard(). Multiple expectDevice() calls
   * accumulate devices for the same board.
   */
  void expectDevice(device::VL53L8CX_Params params);
  void expectDevice(device::TMF8829_Params params);
  void expectDevice(device::HTPA32_Params params);
  void expectDevice(device::WS2812b_Params params);

  /**
   * @brief Declare an expected device by category (matches any sensor of that category).
   *
   * Use these when you don't care which specific sensor is present, only that
   * a device of the given category exists. Default params (or hardware defaults)
   * will be applied to whatever concrete device is discovered.
   */
  void expectDevice(device::AnyDepthSensor_Params params);
  void expectDevice(device::AnyThermalSensor_Params params);
  void expectDevice(device::AnyLight_Params params);

  // ── Default device parameters ──

  /**
   * @brief Set default params applied to every device of the given type that
   *        has no explicit params from expectDevice().
   *
   * May be called multiple times for different device types. Each call
   * replaces any previously set default for that type.
   */
  void setDefaultDeviceParams(device::VL53L8CX_Params params);
  void setDefaultDeviceParams(device::TMF8829_Params params);
  void setDefaultDeviceParams(device::HTPA32_Params params);
  void setDefaultDeviceParams(device::WS2812b_Params params);

  // ── Build ──

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
  /// Concrete device params variant (internal only).
  using ConcreteDeviceParamsVariant = std::variant<device::VL53L8CX_Params, device::HTPA32_Params, device::WS2812b_Params, device::TMF8829_Params>;
  using ConcreteDeviceParamsMap     = std::unordered_map<device::DeviceType, ConcreteDeviceParamsVariant>;

  struct DeviceExpectation {
    device::DeviceType type;
    std::optional<ConcreteDeviceParamsVariant> params; ///< nullopt = use defaults or category match.
  };

  struct BoardExpectation {
    board::SensorBoardParams params;
    std::vector<DeviceExpectation> device_expectations;
    bool has_explicit_devices = false;
  };

  struct InterfaceConfig {
    std::unique_ptr<com::InterfaceParams> params;
    std::vector<BoardExpectation> expected_boards;
    bool has_expectations = false;
  };

  /// Build a ConcreteDeviceParamsMap for the given device types, applying user defaults where available.
  ConcreteDeviceParamsMap buildDefaultParamsMap(const std::vector<device::DeviceType>& devices) const;

  /// Get the current (last) board expectation, or nullptr if none exists.
  BoardExpectation* currentBoardExpectation();

  std::vector<InterfaceConfig> _interfaces;
  ConcreteDeviceParamsMap _default_device_params;
  EnumerationMap _enumeration_results;
  ValidationMode _mode;
};

} // namespace sensorring

} // namespace eduart