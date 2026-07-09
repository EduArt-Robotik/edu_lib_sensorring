#include "board/SensorBoardManager.hpp"

#include <memory>
#include <vector>

#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/board/SensorBoard.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Device.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Device.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Device.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace sensorring {

namespace board {

template <typename DeviceT, typename PrimaryParams, typename FallbackParams> void tryCreateDevice(std::vector<std::unique_ptr<Device> >& devices, const device::DeviceParams* params, com::ComInterfaceID interface, unsigned int idx) {
  if (const auto* p = dynamic_cast<const PrimaryParams*>(params)) {
    devices.push_back(std::make_unique<DeviceT>(*p, interface, idx));
  } else if (const auto* p = dynamic_cast<const FallbackParams*>(params)) {
    devices.push_back(std::make_unique<DeviceT>(*p, interface, idx));
  }
}

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
      logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to create SensorBoard " + std::to_string(enum_info.idx) + " in SensorBoardManager");
      break;
    }
    if (devices.size() > size_before && params.board_type != SensorBoardType::Undefined) {
      devices.back()->setPoseOffset(getDevicePoseOffset(params.board_type, devices.back()->getDeviceID()));
    }
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

std::unique_ptr<SensorBoard> SensorBoardManager::createSensorBoard(EnumerationInformation enum_info, const SensorBoardParams& params, com::ComInterfaceID interface, unsigned int idx, const device::DeviceParamsMap& device_params_map) {
  std::vector<std::unique_ptr<Device> > devices;

  for (const auto& device_type : enum_info.devices) {
    auto it = device_params_map.find(device_type);
    if (it == device_params_map.end()) {
      continue; // Only create devices that are in the params map
    }

    const auto* dev_params        = it->second.get();
    if (dev_params == nullptr || !dev_params->enable) {
      continue;
    }
    const std::size_t size_before = devices.size();

    switch (device_type) {
    case DeviceType::VL53L8CX:
      tryCreateDevice<device::VL53L8CX_Device, device::VL53L8CX_Params, device::DepthSensorParams>(devices, dev_params, interface, idx);
      break;
    case DeviceType::TMF8829:
      tryCreateDevice<device::TMF8829_Device, device::TMF8829_Params, device::DepthSensorParams>(devices, dev_params, interface, idx);
      break;
    case DeviceType::HTPA32:
      tryCreateDevice<device::HTPA32_Device, device::HTPA32_Params, device::ThermalSensorParams>(devices, dev_params, interface, idx);
      break;
    case DeviceType::WS2812b:
      tryCreateDevice<device::WS2812b_Device, device::WS2812b_Params, device::LightParams>(devices, dev_params, interface, idx);
      break;
    default:
      // Unknown or unsupported device type
      break;
    }
    
    if (devices.size() > size_before && params.board_type != SensorBoardType::Undefined) {
      devices.back()->setPoseOffset(getDevicePoseOffset(params.board_type, devices.back()->getDeviceID()));
    } else {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to create SensorBoard " + std::to_string(enum_info.idx) + " in SensorBoardManager");
    }

    // Explicit device types
    // if (const auto* p = dynamic_cast<const device::VL53L8CX_Params*>(dev_params)) {
    //   devices.push_back(std::make_unique<device::VL53L8CX_Device>(*p, interface, idx));
    // } else if (const auto* p = dynamic_cast<const device::HTPA32_Params*>(dev_params)) {
    //   devices.push_back(std::make_unique<device::HTPA32_Device>(*p, interface, idx));
    // } else if (const auto* p = dynamic_cast<const device::WS2812b_Params*>(dev_params)) {
    //   devices.push_back(std::make_unique<device::WS2812b_Device>(*p, interface, idx));
    // } else if (const auto* p = dynamic_cast<const device::TMF8829_Params*>(dev_params)) {
    //   devices.push_back(std::make_unique<device::TMF8829_Device>(*p, interface, idx));
    //   // Base device types
    // } else if (const auto* p = dynamic_cast<const device::DepthSensor_Params*>(dev_params)) {
    //   switch (enum_info.type) {
    //   case DeviceType::VL53L8CX:
    //     devices.push_back(std::make_unique<device::VL53L8CX_Device>(*p, interface, idx));
    //     break;
    //   case DeviceType::TMF8829:
    //     devices.push_back(std::make_unique<device::TMF8829_Device>(*p, interface, idx));
    //     break;
    //   default:
    //     logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to create SensorBoard " + std::to_string(enum_info.idx) + " in SensorBoardManager: Unsupported depth sensor type");
    //     break;
    //   }
    // } else if (const auto* p = dynamic_cast<const device::DepthSensor_Params*>(dev_params)) {
    //   switch (p->getDeviceType()) {
    //   case DeviceType::VL53L8CX:
    //     devices.push_back(std::make_unique<device::VL53L8CX_Device>(*p, interface, idx));
    //     break;
    //   case DeviceType::TMF8829:
    //     devices.push_back(std::make_unique<device::TMF8829_Device>(*p, interface, idx));
    //     break;
    //   default:
    //     logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to create SensorBoard " + std::to_string(enum_info.idx) + " in SensorBoardManager: Unsupported depth sensor type");
    //     break;
    //   }

    // } else {
    //   // Unknown or unsupported device type
    //   logger::Logger::getInstance()->log(logger::LogVerbosity::Exception, "Unable to create SensorBoard " + std::to_string(enum_info.idx) + " in SensorBoardManager");
    // }
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

} // namespace board

} // namespace sensorring

} // namespace eduart
