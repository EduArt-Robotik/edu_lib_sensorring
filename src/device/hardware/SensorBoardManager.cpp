#include "sensorring/device/hardware/SensorBoardManager.hpp"

#include <memory>
#include <vector>

#include "sensorring/SensorBoard.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Params.hpp"

namespace eduart {

namespace device {

std::unique_ptr<SensorBoard> SensorBoardManager::createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx) {
  std::vector<std::unique_ptr<BaseDevice> > devices;

  for (const auto& device_type : enum_info.devices) {
    switch (device_type) {
    case DeviceType::VL53L8CX:
      devices.push_back(std::make_unique<VL53L8CX_Device>(VL53L8CX_Params{}, interface, idx));
      break;
    case DeviceType::HTPA32:
      devices.push_back(std::make_unique<HTPA32_Device>(HTPA32_Params{}, interface, idx));
      break;
    case DeviceType::WS2812b:
      devices.push_back(std::make_unique<WS2812b_Device>(WS2812b_Params{}, interface));
      break;
    default:
      // Unknown or unsupported device type – ignore for now.
      break;
    }
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

std::unique_ptr<SensorBoard> SensorBoardManager::createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx, const DeviceParamsMap& device_params_map) {
  std::vector<std::unique_ptr<BaseDevice> > devices;

  for (const auto& device_type : enum_info.devices) {
    auto it = device_params_map.find(device_type);
    if (it == device_params_map.end()) {
      continue; // Only create devices that are in the params map
    }

    std::visit(
        [&](auto&& device_params) {
          using T = std::decay_t<decltype(device_params)>;
          if constexpr (std::is_same_v<T, VL53L8CX_Params>) {
            devices.push_back(std::make_unique<VL53L8CX_Device>(device_params, interface, idx));
          } else if constexpr (std::is_same_v<T, HTPA32_Params>) {
            devices.push_back(std::make_unique<HTPA32_Device>(device_params, interface, idx));
          } else if constexpr (std::is_same_v<T, WS2812b_Params>) {
            devices.push_back(std::make_unique<WS2812b_Device>(device_params, interface));
          }
        },
        it->second);
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

} // namespace device

} // namespace eduart
