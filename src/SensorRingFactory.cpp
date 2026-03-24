#include "sensorring/SensorRingFactory.hpp"

#include <algorithm>

#include "interface/ComManager.hpp"
#include "sensorring/device/hardware/SensorBoardManager.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace ring {

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void SensorRingFactory::addInterface(com::ComInterfaceID interface) {
  _interfaces.push_back(InterfaceConfig{ interface, {}, false });
}

void SensorRingFactory::expectBoard(device::SensorBoardParams params) {
  if (_interfaces.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "SensorRingFactory::expectBoard called before addInterface.");
    return;
  }
  auto& cfg            = _interfaces.back();
  cfg.has_expectations = true;
  cfg.expected_boards.push_back(BoardExpectation{ params, {}, false });
}

void SensorRingFactory::expectBoard(device::SensorBoardParams params, std::vector<DeviceParamsVariant> device_params) {
  if (_interfaces.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "SensorRingFactory::expectBoard called before addInterface.");
    return;
  }
  auto& cfg            = _interfaces.back();
  cfg.has_expectations = true;
  cfg.expected_boards.push_back(BoardExpectation{ params, std::move(device_params), true });
}

void SensorRingFactory::setDefaultDeviceParams(DeviceParamsVariant params) {
  _default_device_params[deviceTypeFromVariant(params)] = std::move(params);
}

void SensorRingFactory::reset() {
  _interfaces.clear();
  _default_device_params.clear();
}

// ---------------------------------------------------------------------------
// build()  —  enumerate + reconcile + assemble
// ---------------------------------------------------------------------------

std::unique_ptr<SensorRing> SensorRingFactory::build(ValidationMode mode) {
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "SensorRingFactory::build() – starting.");

  const bool strict = (mode == ValidationMode::Strict);

  std::vector<std::unique_ptr<bus::SensorBus> > bus_vec;

  for (auto& iface_cfg : _interfaces) {
    // Obtain the actual interface (may be auto-generated, e.g. USBTINGO).
    auto* iface = com::ComManager::getInstance()->getInterface(iface_cfg.interface);
    if (!iface) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Could not open interface " + iface_cfg.interface.name + " – skipping.");
      continue;
    }
    auto id = iface->getID();

    // ── Enumerate ──
    auto enum_infos = bus::SensorBus::queryConnectedDevices(id);
    if (enum_infos.empty()) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "No boards found on interface " + id.name + ".");
      if (iface_cfg.has_expectations) {
        if (strict) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Expected " + std::to_string(iface_cfg.expected_boards.size()) + " board(s) on " + id.name + " but found none.");
          return nullptr;
        }
        logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Expected " + std::to_string(iface_cfg.expected_boards.size()) + " board(s) on " + id.name + " but found none – skipping interface (relaxed mode).");
      }
      continue;
    }

    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Found " + std::to_string(enum_infos.size()) + " board(s) on " + id.name + ".");

    std::vector<std::unique_ptr<device::SensorBoard> > board_vec;
    board_vec.reserve(enum_infos.size());

    if (!iface_cfg.has_expectations) {
      // ── Auto-discovery mode ──
      for (const auto& enum_info : enum_infos) {
        unsigned int idx = (enum_info.idx > 0u) ? enum_info.idx - 1u : 0u;
        device::SensorBoardParams board_params;
        board_params.board_type = enum_info.type;

        if (_default_device_params.empty()) {
          board_vec.push_back(device::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx));
        } else {
          // Build a params map from defaults for the devices the hardware reports.
          device::SensorBoardManager::DeviceParamsMap params_map;
          for (const auto& dev_type : enum_info.devices) {
            auto def_it = _default_device_params.find(dev_type);
            if (def_it != _default_device_params.end()) {
              params_map[dev_type] = def_it->second;
            } else {
              // Use default-constructed params for device types without user defaults.
              switch (dev_type) {
              case device::DeviceType::VL53L8CX:
                params_map[dev_type] = device::VL53L8CX_Params{};
                break;
              case device::DeviceType::HTPA32:
                params_map[dev_type] = device::HTPA32_Params{};
                break;
              case device::DeviceType::WS2812b:
                params_map[dev_type] = device::WS2812b_Params{};
                break;
              default:
                break;
              }
            }
          }
          board_vec.push_back(device::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
        }
      }
    } else {
      // ── Configured mode ──

      // Validate board count.
      if (enum_infos.size() != iface_cfg.expected_boards.size()) {
        if (strict) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Board count mismatch on " + id.name + ": expected " + std::to_string(iface_cfg.expected_boards.size()) + ", found " + std::to_string(enum_infos.size()) + ".");
          return nullptr;
        }
        logger::Logger::getInstance()->log(
            logger::LogVerbosity::Warning,
            "Board count mismatch on " + id.name + ": expected " + std::to_string(iface_cfg.expected_boards.size()) + ", found " + std::to_string(enum_infos.size()) + " – reconciling available boards (relaxed mode).");
      }

      // Iterate only over boards that have both an expectation and a discovered counterpart.
      const auto board_count = std::min(enum_infos.size(), iface_cfg.expected_boards.size());

      for (std::size_t i = 0; i < board_count; ++i) {
        const auto& enum_info   = enum_infos[i];
        const auto& expectation = iface_cfg.expected_boards[i];
        unsigned int idx        = (enum_info.idx > 0u) ? enum_info.idx - 1u : 0u;

        // Validate board type if specified.
        if (expectation.params.board_type != device::SensorBoardType::Undefined && expectation.params.board_type != enum_info.type) {
          if (strict) {
            logger::Logger::getInstance()->log(
                logger::LogVerbosity::Error, "Board type mismatch at index " + std::to_string(i) + " on " + id.name + ": expected " + device::toString(expectation.params.board_type) + ", found " + device::toString(enum_info.type) + ".");
            return nullptr;
          }
          logger::Logger::getInstance()->log(
              logger::LogVerbosity::Warning, "Board type mismatch at index " + std::to_string(i) + " on " + id.name + ": expected " + device::toString(expectation.params.board_type) + ", found " + device::toString(enum_info.type)
                                                 + " – skipping board (relaxed mode).");
          continue;
        }

        // Use the enumerated board type when the user didn't constrain it.
        device::SensorBoardParams board_params = expectation.params;
        if (board_params.board_type == device::SensorBoardType::Undefined) {
          board_params.board_type = enum_info.type;
        }

        if (expectation.has_explicit_devices) {
          // Build a params map from the user-provided device params.
          device::SensorBoardManager::DeviceParamsMap params_map;
          for (const auto& dp : expectation.device_params) {
            auto dt        = deviceTypeFromVariant(dp);
            params_map[dt] = dp;
          }

          // Validate that the hardware has all requested device types.
          bool devices_ok = true;
          for (const auto& [required_type, _] : params_map) {
            if (std::find(enum_info.devices.begin(), enum_info.devices.end(), required_type) == enum_info.devices.end()) {
              if (strict) {
                logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Board at index " + std::to_string(i) + " on " + id.name + " does not have required device type " + device::toString(required_type) + ".");
                return nullptr;
              }
              logger::Logger::getInstance()->log(
                  logger::LogVerbosity::Warning,
                  "Board at index " + std::to_string(i) + " on " + id.name + " does not have required device type " + device::toString(required_type) + " – skipping board (relaxed mode).");
              devices_ok = false;
              break;
            }
          }
          if (!devices_ok) {
            continue;
          }

          board_vec.push_back(device::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
        } else {
          // No explicit device params → use all devices from hardware, apply defaults where available.
          device::SensorBoardManager::DeviceParamsMap params_map;
          for (const auto& dev_type : enum_info.devices) {
            auto def_it = _default_device_params.find(dev_type);
            if (def_it != _default_device_params.end()) {
              params_map[dev_type] = def_it->second;
            } else {
              switch (dev_type) {
              case device::DeviceType::VL53L8CX:
                params_map[dev_type] = device::VL53L8CX_Params{};
                break;
              case device::DeviceType::HTPA32:
                params_map[dev_type] = device::HTPA32_Params{};
                break;
              case device::DeviceType::WS2812b:
                params_map[dev_type] = device::WS2812b_Params{};
                break;
              default:
                break;
              }
            }
          }
          board_vec.push_back(device::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
        }
      }
    }

    if (!board_vec.empty()) {
      bus_vec.push_back(std::make_unique<bus::SensorBus>(id, std::move(board_vec)));
    }
  }

  if (bus_vec.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "No boards found on any interface. Failed to create SensorRing.");
    return nullptr;
  }

  auto ring = std::make_unique<SensorRing>(std::move(bus_vec));
  reset();
  return ring;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

device::DeviceType SensorRingFactory::deviceTypeFromVariant(const DeviceParamsVariant& v) {
  return std::visit(
      [](auto&& arg) -> device::DeviceType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, device::VL53L8CX_Params>) {
          return device::DeviceType::VL53L8CX;
        } else if constexpr (std::is_same_v<T, device::HTPA32_Params>) {
          return device::DeviceType::HTPA32;
        } else if constexpr (std::is_same_v<T, device::WS2812b_Params>) {
          return device::DeviceType::WS2812b;
        }
        return device::DeviceType::UNDEFINED;
      },
      v);
}

} // namespace ring

} // namespace eduart