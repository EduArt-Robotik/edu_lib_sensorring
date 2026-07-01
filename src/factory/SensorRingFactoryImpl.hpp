#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/depth/DepthSensorParams.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/light/LightParams.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/device/thermal/ThermalSensorParams.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/interface/InterfaceParams.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {
namespace sensorring {

class SensorRingFactoryImpl {
public:
  using EnumerationMap = SensorRingFactory::EnumerationMap;

  struct DeviceExpectation {
    device::DeviceType type                      = device::DeviceType::Undefined;
    std::shared_ptr<device::DeviceParams> params = nullptr;
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

  struct ResolvedDeviceConfig {
    device::DeviceParamsMap params_map;
    std::vector<device::DeviceType> configured_devs;
  };

  enum class BuildStepStatus {
    Continue,
    Skip,
    Fatal
  };

  explicit SensorRingFactoryImpl(ValidationMode mode = ValidationMode::Relaxed);

  void addInterface(com::SocketCanParams params);
  void addInterface(com::UsbTingoParams params);

  void expectBoard(board::SensorBoardParams params = {});
  void expectDevice(device::VL53L8CX_Params params);
  void expectDevice(device::TMF8829_Params params);
  void expectDevice(device::HTPA32_Params params);
  void expectDevice(device::WS2812b_Params params);
  void expectDevice(device::DepthSensorParams params);
  void expectDevice(device::ThermalSensorParams params);
  void expectDevice(device::LightParams params);

  void setDefaultDeviceParams(device::VL53L8CX_Params params);
  void setDefaultDeviceParams(device::TMF8829_Params params);
  void setDefaultDeviceParams(device::HTPA32_Params params);
  void setDefaultDeviceParams(device::WS2812b_Params params);
  void setDefaultDeviceParams(device::DepthSensorParams params);
  void setDefaultDeviceParams(device::ThermalSensorParams params);
  void setDefaultDeviceParams(device::LightParams params);

  void reset();

  std::unique_ptr<SensorRing> build();
  EnumerationMap enumerate();
  const EnumerationMap& getLatestEnumerationResult() const;
  std::string printTopology() const;

  // Internal seam for unit tests only.
  void setEnumerationResultsForTest(EnumerationMap results);

  device::DeviceParamsMap buildDefaultParamsMap(const std::vector<device::DeviceType>& devices) const;
  ResolvedDeviceConfig resolveDeviceExpectations(const std::vector<DeviceExpectation>& device_expectations, const std::vector<device::DeviceType>& available_devices) const;
  BuildStepStatus validateFirmware(std::vector<board::EnumerationInformation>& enum_infos, const com::ComInterfaceID& id, bool strict) const;

private:
  BuildStepStatus enumerateInterface(const InterfaceConfig& iface_cfg, bool strict, com::ComInterfaceID& id, std::vector<board::EnumerationInformation>& enum_infos) const;
  bool processInterfaceBuild(const InterfaceConfig& iface_cfg, bool strict, std::vector<std::unique_ptr<SensorBus> >& bus_vec, EnumerationMap& enumeration_map) const;
  void buildAutoDiscoveredBoards(
      const com::ComInterfaceID& id, std::vector<board::EnumerationInformation>& enum_infos, std::vector<std::unique_ptr<board::SensorBoard> >& board_vec, std::vector<board::EnumerationInformation>& enriched_enum) const;
  bool buildConfiguredStrictBoards(
      const InterfaceConfig& iface_cfg, const com::ComInterfaceID& id, std::vector<board::EnumerationInformation>& enum_infos, std::vector<std::unique_ptr<board::SensorBoard> >& board_vec,
      std::vector<board::EnumerationInformation>& enriched_enum) const;
  void buildConfiguredRelaxedBoards(
      const InterfaceConfig& iface_cfg, const com::ComInterfaceID& id, std::vector<board::EnumerationInformation>& enum_infos, std::vector<std::unique_ptr<board::SensorBoard> >& board_vec,
      std::vector<board::EnumerationInformation>& enriched_enum) const;
  bool createBoardFromExpectation(
      const BoardExpectation& expectation, board::EnumerationInformation& enum_info, const board::SensorBoardParams& board_params, const com::ComInterfaceID& id, unsigned int idx,
      std::vector<std::unique_ptr<board::SensorBoard> >& board_vec, std::optional<std::size_t> strict_board_index = std::nullopt) const;

  BoardExpectation* currentBoardExpectation();

  template <typename Params> void expectDeviceImpl(device::DeviceType type, Params params) {
    auto* board = currentBoardExpectation();
    if (!board) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "SensorRingFactory::expectDevice called before expectBoard.");
      return;
    }

    board->has_explicit_devices = true;
    board->device_expectations.push_back({ type, std::make_unique<Params>(std::move(params)) });
  }

  std::vector<InterfaceConfig> _interfaces;
  device::DeviceParamsMap _default_device_params;
  EnumerationMap _enumeration_results;
  ValidationMode _mode;
};

} // namespace sensorring
} // namespace eduart
