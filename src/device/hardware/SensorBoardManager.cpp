#include "sensorring/device/hardware/SensorBoardManager.hpp"

#include <memory>
#include <vector>

#include "sensorring/Parameter.hpp"
#include "sensorring/SensorBoard.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"

namespace eduart {

namespace device {

std::unique_ptr<SensorBoard> SensorBoardManager::createSensorBoard(SensorBoardType board_type, const SensorBoardParams& params, com::ComInterface* interface, unsigned int idx) {
  std::vector<std::unique_ptr<BaseDevice> > devices;

  // Backwards-compatible behaviour: if no concrete board type is specified,
  // instantiate all supported devices like the legacy SensorRing::create().
  if (board_type == SensorBoardType::Undefined) {
    devices.push_back(std::make_unique<VL53L8CX_Device>(params.vl53l8cx_params, interface, idx));
    devices.push_back(std::make_unique<HTPA32_Device>(params.htpa32_params, interface, idx));
    devices.push_back(std::make_unique<WS2812b_Device>(params.ws2812b_params, interface));
  } else {
    const auto& board_info = getSensorBoardInfo(board_type);

    // Create only the devices that are defined for this concrete hardware board.
    for (const auto& dev : board_info.devices) {
      switch (dev.id.getType()) {
      case DeviceType::VL53L8CX:
        devices.push_back(std::make_unique<VL53L8CX_Device>(params.vl53l8cx_params, interface, idx));
        break;
      case DeviceType::HTPA32:
        devices.push_back(std::make_unique<HTPA32_Device>(params.htpa32_params, interface, idx));
        break;
      case DeviceType::WS2812b:
        devices.push_back(std::make_unique<WS2812b_Device>(params.ws2812b_params, interface));
        break;
      default:
        // Unknown or unsupported device type – ignore for now.
        break;
      }
    }
  }

  return std::make_unique<SensorBoard>(params, interface, idx, std::move(devices));
}

} // namespace device

} // namespace eduart
