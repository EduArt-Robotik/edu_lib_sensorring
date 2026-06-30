#include "sensorring/SensorRingFactory.hpp"

#include <algorithm>
#include <sstream>

#include "board/SensorBoardManager.hpp"
#include "interface/ComManager.hpp"
#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensorring {

namespace {
/// Helper: obtain the ComInterface* from an InterfaceParams via ComManager.
com::ComInterface* openInterface(const com::InterfaceParams& params) {
  if (auto* p = dynamic_cast<const com::SocketCanParams*>(&params)) {
    return com::ComManager::getInstance()->getInterface(*p);
  }
  if (auto* p = dynamic_cast<const com::UsbTingoParams*>(&params)) {
    return com::ComManager::getInstance()->getInterface(*p);
  }
  return nullptr;
}

} // namespace

SensorRingFactory::SensorRingFactory(ValidationMode mode)
    : _mode(mode) {
}

// ── Interface configuration ──

void SensorRingFactory::addInterface(com::SocketCanParams params) {
  _interfaces.push_back(InterfaceConfig{ std::make_unique<com::SocketCanParams>(std::move(params)), {}, false });
}

void SensorRingFactory::addInterface(com::UsbTingoParams params) {
  _interfaces.push_back(InterfaceConfig{ std::make_unique<com::UsbTingoParams>(std::move(params)), {}, false });
}

// ── Board expectations ──

void SensorRingFactory::expectBoard(board::SensorBoardParams params) {
  if (_interfaces.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "SensorRingFactory::expectBoard called before addInterface.");
    return;
  }
  auto& cfg            = _interfaces.back();
  cfg.has_expectations = true;
  cfg.expected_boards.push_back(BoardExpectation{ std::move(params), {}, false });
}

// ── Device expectations ──

SensorRingFactory::BoardExpectation* SensorRingFactory::currentBoardExpectation() {
  if (_interfaces.empty()) {
    return nullptr;
  }
  auto& boards = _interfaces.back().expected_boards;
  if (boards.empty()) {
    return nullptr;
  }
  return &boards.back();
}

void SensorRingFactory::expectDevice(device::VL53L8CX_Params params) {
  expectDeviceImpl<device::VL53L8CX_Params>(device::DeviceType::VL53L8CX, std::move(params));
}

void SensorRingFactory::expectDevice(device::TMF8829_Params params) {
  expectDeviceImpl<device::TMF8829_Params>(device::DeviceType::TMF8829, std::move(params));
}

void SensorRingFactory::expectDevice(device::HTPA32_Params params) {
  expectDeviceImpl<device::HTPA32_Params>(device::DeviceType::HTPA32, std::move(params));
}

void SensorRingFactory::expectDevice(device::WS2812b_Params params) {
  expectDeviceImpl<device::WS2812b_Params>(device::DeviceType::WS2812b, std::move(params));
}

void SensorRingFactory::expectDevice(device::DepthSensorParams params) {
  expectDeviceImpl<device::DepthSensorParams>(device::DeviceType::AnyDepth, std::move(params));
}

void SensorRingFactory::expectDevice(device::ThermalSensorParams params) {
  expectDeviceImpl<device::ThermalSensorParams>(device::DeviceType::AnyThermal, std::move(params));
}

void SensorRingFactory::expectDevice(device::LightParams params) {
  expectDeviceImpl<device::LightParams>(device::DeviceType::AnyLight, std::move(params));
}


// ── Default device parameters ──

void SensorRingFactory::setDefaultDeviceParams(device::VL53L8CX_Params params) {
  auto p                                               = std::make_unique<device::VL53L8CX_Params>(std::move(params));
  _default_device_params[device::DeviceType::VL53L8CX] = std::move(p);
}

void SensorRingFactory::setDefaultDeviceParams(device::TMF8829_Params params) {
  auto p                                              = std::make_unique<device::TMF8829_Params>(std::move(params));
  _default_device_params[device::DeviceType::TMF8829] = std::move(p);
}

void SensorRingFactory::setDefaultDeviceParams(device::HTPA32_Params params) {
  auto p                                             = std::make_unique<device::HTPA32_Params>(std::move(params));
  _default_device_params[device::DeviceType::HTPA32] = std::move(p);
}

void SensorRingFactory::setDefaultDeviceParams(device::WS2812b_Params params) {
  auto p                                              = std::make_unique<device::WS2812b_Params>(std::move(params));
  _default_device_params[device::DeviceType::WS2812b] = std::move(p);
}

// ── Reset ──

void SensorRingFactory::reset() {
  _interfaces.clear();
  _default_device_params.clear();
}

std::unique_ptr<SensorRing> SensorRingFactory::build() {
  const auto mode = _mode;
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "SensorRingFactory::build() - starting.");

  const bool strict = (mode == ValidationMode::Strict);

  std::vector<std::unique_ptr<SensorBus> > bus_vec;
  EnumerationMap enumeration_map;

  for (auto& iface_cfg : _interfaces) {
    // Obtain the actual interface (may be auto-generated, e.g. USBTINGO).
    try {
      auto* iface = openInterface(*iface_cfg.params);
      if (!iface) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Could not open interface " + iface_cfg.params->name + " - skipping.");
        continue;
      }
      auto id = iface->getID();

      // ── Enumerate ──
      auto enum_infos = SensorBus::queryConnectedDevices(id);
      if (enum_infos.empty()) {
        logger::Logger::getInstance()->log(logger::LogVerbosity::Info, "No boards found on interface " + id.name + ".");
        if (iface_cfg.has_expectations) {
          if (strict) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Expected " + std::to_string(iface_cfg.expected_boards.size()) + " board(s) on " + id.name + " but found none.");
            return nullptr;
          }
          logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Expected " + std::to_string(iface_cfg.expected_boards.size()) + " board(s) on " + id.name + " but found none - skipping interface (relaxed mode).");
        }
        continue;
      }

      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Found " + std::to_string(enum_infos.size()) + " board(s) on " + id.name + ".");

      // Mark all discovered boards as Connected and check firmware compatibility.
      bool firmware_ok = true;
      for (auto& ei : enum_infos) {
        ei.state = board::ConnectionState::Connected;

        if (ei.version < MIN_FIRMWARE_VERSION) {
          logger::Logger::getInstance()->log(
              logger::LogVerbosity::Error, "Board " + std::to_string(ei.idx) + " on " + id.name + " has firmware " + ei.version.toString() + " but minimum required is " + MIN_FIRMWARE_VERSION.toString() + ".");
          firmware_ok = false;
        }
      }

      if (!firmware_ok) {
        if (strict) {
          return nullptr;
        }
        logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "One or more boards on " + id.name + " have incompatible firmware - skipping interface (relaxed mode).");
        continue;
      }

      std::vector<std::unique_ptr<board::SensorBoard> > board_vec;
      board_vec.reserve(enum_infos.size());

      // Enriched enumeration results: one entry per discovered board, annotated
      // with ConfigurationState and configured_devices.
      std::vector<board::EnumerationInformation> enriched_enum;
      enriched_enum.reserve(enum_infos.size());

      if (!iface_cfg.has_expectations) {
        // ── Auto-discovery mode ──
        // All discovered boards are Unconfigured (no user expectations).
        for (auto& enum_info : enum_infos) {
          unsigned int idx = enum_info.idx;
          board::SensorBoardParams board_params;
          board_params.board_type = enum_info.type;

          enum_info.config_state       = board::ConfigurationState::Unconfigured;
          enum_info.configured_devices = enum_info.devices; // all devices used

          if (_default_device_params.empty()) {
            board_vec.push_back(board::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx));
          } else {
            auto params_map = buildDefaultParamsMap(enum_info.devices);
            board_vec.push_back(board::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
          }
          enriched_enum.push_back(enum_info);
        }
      } else {
        // ── Configured mode ──

        if (strict) {
          // ── Strict: index-ordered 1:1 matching ──

          // Validate board count.
          if (enum_infos.size() != iface_cfg.expected_boards.size()) {
            logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Board count mismatch on " + id.name + ": expected " + std::to_string(iface_cfg.expected_boards.size()) + ", found " + std::to_string(enum_infos.size()) + ".");
            return nullptr;
          }

          for (std::size_t i = 0; i < enum_infos.size(); ++i) {
            auto& enum_info         = enum_infos[i];
            const auto& expectation = iface_cfg.expected_boards[i];
            unsigned int idx        = enum_info.idx;

            // Validate board type if specified.
            if (expectation.params.board_type != board::SensorBoardType::Undefined && expectation.params.board_type != enum_info.type) {
              logger::Logger::getInstance()->log(
                  logger::LogVerbosity::Error, "Board type mismatch at index " + std::to_string(i) + " on " + id.name + ": expected " + board::toString(expectation.params.board_type) + ", found " + board::toString(enum_info.type) + ".");
              return nullptr;
            }

            board::SensorBoardParams board_params = expectation.params;
            if (board_params.board_type == board::SensorBoardType::Undefined) {
              board_params.board_type = enum_info.type;
            }

            if (expectation.has_explicit_devices) {

              device::DeviceParamsMap params_map;
              std::vector<device::DeviceType> configured_devs;
              for (const auto& de : expectation.device_expectations) {

                auto matched_type        = de.type;
                const auto original_type = de.type;
                if (device::isCategory(de.type)) {
                  // If device is a category take the first compatible device on the board
                  for (auto actual_dt : enum_info.devices) {
                    if (device::deviceMatchesExpected(actual_dt, de.type)) {
                      matched_type = actual_dt;
                      break;
                    }
                  }
                } else {
                  matched_type = de.type;
                }

                if (de.params) {
                  params_map[matched_type] = de.params;
                } else {
                  auto resolved = buildDefaultParamsMap({ matched_type });
                  if (auto it = resolved.find(original_type); it != resolved.end()) {
                    params_map[matched_type] = it->second;
                  }
                }
                configured_devs.push_back(matched_type);
              }

              for (const auto& [required_type, _] : params_map) {
                if (std::find(enum_info.devices.begin(), enum_info.devices.end(), required_type) == enum_info.devices.end()) {
                  logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Board at index " + std::to_string(i) + " on " + id.name + " does not have required device type " + device::toString(required_type) + ".");
                  return nullptr;
                }
              }

              enum_info.config_state       = board::ConfigurationState::Configured;
              enum_info.configured_devices = std::move(configured_devs);
              board_vec.push_back(board::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
            } else {
              auto params_map = buildDefaultParamsMap(enum_info.devices);

              enum_info.config_state       = board::ConfigurationState::Configured;
              enum_info.configured_devices = enum_info.devices;
              board_vec.push_back(board::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
            }
            enriched_enum.push_back(enum_info);
          }
        } else {
          // ── Relaxed: search-based matching ──
          // For each expectation, search through unclaimed boards for the first
          // compatible one. This allows expectations to match boards regardless
          // of their physical position on the bus.

          if (enum_infos.size() != iface_cfg.expected_boards.size()) {
            logger::Logger::getInstance()->log(
                logger::LogVerbosity::Warning,
                "Board count mismatch on " + id.name + ": expected " + std::to_string(iface_cfg.expected_boards.size()) + ", found " + std::to_string(enum_infos.size()) + " - searching for compatible boards (relaxed mode).");
          }

          std::vector<bool> claimed(enum_infos.size(), false);

          for (std::size_t exp_i = 0; exp_i < iface_cfg.expected_boards.size(); ++exp_i) {
            const auto& expectation = iface_cfg.expected_boards[exp_i];
            bool matched            = false;

            for (std::size_t j = 0; j < enum_infos.size(); ++j) {
              if (claimed[j]) {
                continue;
              }

              auto& enum_info = enum_infos[j];

              // Check board type constraint.
              if (expectation.params.board_type != board::SensorBoardType::Undefined && expectation.params.board_type != enum_info.type) {
                continue;
              }

              // Check required device types (only when explicit devices are specified).
              if (expectation.has_explicit_devices) {
                bool has_all = true;
                for (const auto& de : expectation.device_expectations) {
                  bool found = std::any_of(enum_info.devices.begin(), enum_info.devices.end(), [&de](const device::DeviceType actual) {
                    return device::deviceMatchesExpected(actual, de.type);
                  });
                  if (!found) {
                    has_all = false;
                    break;
                  }
                }
                if (!has_all) {
                  continue;
                }
              }

              // ── Match found ──
              claimed[j]       = true;
              matched          = true;
              unsigned int idx = enum_info.idx;

              board::SensorBoardParams board_params = expectation.params;
              if (board_params.board_type == board::SensorBoardType::Undefined) {
                board_params.board_type = enum_info.type;
              }

              if (expectation.has_explicit_devices) {
                device::DeviceParamsMap params_map;
                std::vector<device::DeviceType> configured_devs;
                for (const auto& de : expectation.device_expectations) {

                  auto matched_type        = de.type;
                  const auto original_type = de.type;
                  if (device::isCategory(de.type)) {
                    // If device is a category take the first compatible device on the board
                    for (auto actual_dt : enum_info.devices) {
                      if (device::deviceMatchesExpected(actual_dt, de.type)) {
                        matched_type = actual_dt;
                        break;
                      }
                    }
                  } else {
                    matched_type = de.type;
                  }

                  if (de.params) {
                    params_map[matched_type] = de.params;
                  } else {
                    auto resolved = buildDefaultParamsMap({ matched_type });
                    if (auto it = resolved.find(original_type); it != resolved.end()) { // If type was a category, look up the original type for consitency
                      params_map[matched_type] = it->second;
                    }
                  }
                  configured_devs.push_back(matched_type);
                }

                enum_info.config_state       = board::ConfigurationState::Configured;
                enum_info.configured_devices = std::move(configured_devs);
                board_vec.push_back(board::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
              } else {
                auto params_map = buildDefaultParamsMap(enum_info.devices);

                enum_info.config_state       = board::ConfigurationState::Configured;
                enum_info.configured_devices = enum_info.devices;
                board_vec.push_back(board::SensorBoardManager::createSensorBoard(enum_info, board_params, id, idx, params_map));
              }
              break;
            }

            if (!matched) {
              logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "No compatible board found for expectation " + std::to_string(exp_i) + " on " + id.name + " - skipping (relaxed mode).");

              board::EnumerationInformation unconnected;
              unconnected.idx          = static_cast<unsigned int>(exp_i);
              unconnected.state        = board::ConnectionState::Unconnected;
              unconnected.config_state = board::ConfigurationState::Configured;
              unconnected.type         = expectation.params.board_type;
              enriched_enum.push_back(unconnected);
            }
          }

          // Record all discovered boards in enrichment order (matched ones already
          // have their config_state set; unclaimed ones are marked unconfigured).
          for (std::size_t j = 0; j < enum_infos.size(); ++j) {
            if (!claimed[j]) {
              enum_infos[j].config_state = board::ConfigurationState::Unconfigured;
            }
            enriched_enum.push_back(enum_infos[j]);
          }
        }
      }

      if (!board_vec.empty()) {
        enumeration_map[id] = std::move(enriched_enum);
        bus_vec.push_back(std::make_unique<SensorBus>(id, std::move(board_vec)));
      }
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Error while opening interface " + iface_cfg.params->name + " - skipping.");
      continue;
    }
  }

  if (bus_vec.empty()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "No boards found on any interface. Failed to create SensorRing.");
    return nullptr;
  }

  _enumeration_results = std::move(enumeration_map);

  auto ring = std::make_unique<SensorRing>(std::move(bus_vec));

  // Assign globally unique per-type device indices across all buses.
  {
    std::unordered_map<int, unsigned int> type_counters;
    for (auto* dev : ring->getDevices()) {
      auto type_key    = static_cast<int>(dev->getDeviceID().getType());
      unsigned int seq = type_counters[type_key]++;
      dev->setDeviceIndex(seq);
    }
  }

  // Print topology summary via the logger.
  logger::Logger::getInstance()->log(logger::LogVerbosity::Info, printTopology());

  reset();
  return ring;
}

SensorRingFactory::EnumerationMap SensorRingFactory::enumerate() {
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "SensorRingFactory::enumerate() - scanning interfaces.");

  EnumerationMap result;

  for (const auto& iface_cfg : _interfaces) {
    try {
      auto* iface = openInterface(*iface_cfg.params);
      if (!iface) {
        continue;
      }
      auto id         = iface->getID();
      auto enum_infos = SensorBus::queryConnectedDevices(id);

      for (auto& ei : enum_infos) {
        ei.state        = board::ConnectionState::Connected;
        ei.config_state = board::ConfigurationState::Undefined;
      }

      result[id] = std::move(enum_infos);
    } catch (const std::exception& e) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Error while enumerating interface " + iface_cfg.params->name + " - skipping.");
    }
  }

  _enumeration_results = result;
  return result;
}

const SensorRingFactory::EnumerationMap& SensorRingFactory::getLatestEnumerationResult() const {
  return _enumeration_results;
}

std::string SensorRingFactory::printTopology() const {
  std::stringstream ss;

  for (const auto& [iface_id, boards] : _enumeration_results) {
    ss << std::endl << std::endl;
    ss << "=================================================" << std::endl;
    ss << "Topology of the sensors on " << iface_id.name << ":" << std::endl;
    ss << std::endl;

    for (const auto& board : boards) {
      ss << board.toString() << std::endl;
    }

    ss << "=================================================" << std::endl;
  }

  return ss.str();
}

device::DeviceParamsMap SensorRingFactory::buildDefaultParamsMap(const std::vector<device::DeviceType>& devices) const {
  device::DeviceParamsMap params_map;
  for (const auto& dev_type : devices) {
    auto def_it = _default_device_params.find(dev_type);
    if (def_it != _default_device_params.end()) {
      params_map[dev_type] = def_it->second;
    } else {
      switch (dev_type) {
      case device::DeviceType::VL53L8CX:
        params_map[dev_type] = std::make_shared<device::VL53L8CX_Params>();
        break;
      case device::DeviceType::HTPA32:
        params_map[dev_type] = std::make_shared<device::HTPA32_Params>();
        break;
      case device::DeviceType::WS2812b:
        params_map[dev_type] = std::make_shared<device::WS2812b_Params>();
        break;
      case device::DeviceType::TMF8829:
        params_map[dev_type] = std::make_shared<device::TMF8829_Params>();
        break;
      default:
        break;
      }
    }
  }
  return params_map;
}

/// Device expectation logic
template <typename Params> void SensorRingFactory::expectDeviceImpl(device::DeviceType type, Params params) {
  auto* board = currentBoardExpectation();
  if (!board) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "SensorRingFactory::expectDevice called before expectBoard.");
    return;
  }

  board->has_explicit_devices = true;
  board->device_expectations.push_back({ type, std::make_unique<Params>(std::move(params)) });
}

} // namespace sensorring

} // namespace eduart