#include "board/SensorBoardManager.hpp"

#include <memory>
#include <vector>

#include "sensorring/board/SensorBoard.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Device.hpp"

namespace eduart {

namespace sensorring {

namespace board {

std::unique_ptr<SensorBoard> SensorBoardManager::createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx) {
  std::vector<std::unique_ptr<Device> > devices;

  for (const auto& device_type : enum_info.devices) {
    const std::size_t size_before = devices.size();
    switch (device_type) {
    case DeviceType::VL53L8CX:
      devices.push_back(std::make_unique<device::VL53L8CX_Device>(device::VL53L8CX_Params{}, interface, idx));
      break;
    case DeviceType::HTPA32:
      devices.push_back(std::make_unique<device::HTPA32_Device>(device::HTPA32_Params{}, interface, idx));
      break;
    case DeviceType::WS2812b:
      devices.push_back(std::make_unique<device::WS2812b_Device>(device::WS2812b_Params{}, interface, idx));
      break;
    case DeviceType::TMF8829:
      devices.push_back(std::make_unique<device::TMF8829_Device>(device::TMF8829_Params{}, interface, idx));
      break;
    default:
      // Unknown or unsupported device type
      break;
    }
    if (devices.size() > size_before && params.board_type != SensorBoardType::Undefined) {
      devices.back()->setPoseOffset(getDevicePoseOffset(params.board_type, devices.back()->getDeviceID()));
    }
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

std::unique_ptr<SensorBoard> SensorBoardManager::createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx, const DeviceParamsMap& device_params_map) {
  std::vector<std::unique_ptr<Device> > devices;

  for (const auto& device_type : enum_info.devices) {
    auto it = device_params_map.find(device_type);
    if (it == device_params_map.end()) {
      continue; // Only create devices that are in the params map
    }

    std::visit(
        [&](auto&& device_params) {
          using T = std::decay_t<decltype(device_params)>;
          if constexpr (std::is_same_v<T, device::VL53L8CX_Params>) {
            devices.push_back(std::make_unique<device::VL53L8CX_Device>(device_params, interface, idx));
          } else if constexpr (std::is_same_v<T, device::HTPA32_Params>) {
            devices.push_back(std::make_unique<device::HTPA32_Device>(device_params, interface, idx));
          } else if constexpr (std::is_same_v<T, device::WS2812b_Params>) {
            devices.push_back(std::make_unique<device::WS2812b_Device>(device_params, interface, idx));
          } else if constexpr (std::is_same_v<T, device::TMF8829_Params>) {
            devices.push_back(std::make_unique<device::TMF8829_Device>(device_params, interface, idx));
          }
        },
        it->second);
    if (params.board_type != SensorBoardType::Undefined) {
      devices.back()->setPoseOffset(getDevicePoseOffset(params.board_type, devices.back()->getDeviceID()));
    }
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

} // namespace board

} // namespace sensorring

} // namespace eduart
