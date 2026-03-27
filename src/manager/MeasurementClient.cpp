#include "sensorring/manager/MeasurementClient.hpp"

#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/manager/ManagerState.hpp"

namespace eduart {

namespace manager {

MeasurementClient::~MeasurementClient() {
  unregisterClient();
}

bool MeasurementClient::registerClient(MeasurementManager* manager) {
  if (!manager) {
    return false;
  }

  if (_managers.find(manager) == _managers.end()) {
    _managers.insert(manager);
    auto& subs = _subscriptions[manager];
    subs.emplace_back(manager->subscribeToStateChanges(std::bind(&MeasurementClient::onStateChange, this, std::placeholders::_1)));
    subs.emplace_back(manager->subscribeToDeviceGroup(device::DeviceType::VL53L8CX, std::bind(&MeasurementClient::onTofDispatcher, this, std::placeholders::_1)));
    subs.emplace_back(manager->subscribeToDeviceGroup(device::DeviceType::HTPA32, std::bind(&MeasurementClient::onThermalDispatcher, this, std::placeholders::_1)));
    return true;
  }
  return false;
}

bool MeasurementClient::unregisterClient() {
  bool success = false;
  for (auto manager : _managers) {
    if (manager) {
      _subscriptions.erase(manager);
      success = true;
    }
  }
  _managers.clear();
  return success;
}

bool MeasurementClient::unregisterClient(MeasurementManager* manager) {
  if (!manager) {
    return false;
  }
  auto it = _managers.find(manager);
  if (it == _managers.end()) {
    return false;
  }
  _subscriptions.erase(manager);
  _managers.erase(it);
  return true;
}

void MeasurementClient::onTofDispatcher(const device::DeviceGroup& group) {
  std::vector<measurement::TofMeasurement> raw_measurement_vec;
  std::vector<measurement::TofMeasurement> transformed_measurement_vec;

  group.invokeForEachDeviceOfType<device::VL53L8CX_Device>([&raw_measurement_vec, &transformed_measurement_vec](device::VL53L8CX_Device* device) {
    if (!device->getEnable())
      return;
    auto [raw_meas, raw_state] = device->getLatestMeasurement();
    if (raw_state == device::SensorState::SensorOK) {
      if (!raw_meas.point_cloud.data.empty())
        raw_measurement_vec.emplace_back(raw_meas);
    }
    auto [trans_meas, trans_state] = device->getLatestTransformedMeasurement();
    if (trans_state == device::SensorState::SensorOK) {
      if (!trans_meas.point_cloud.data.empty())
        transformed_measurement_vec.emplace_back(trans_meas);
    }
  });

  if (!raw_measurement_vec.empty()) {
    onRawTofMeasurement(raw_measurement_vec);
  }
  if (!transformed_measurement_vec.empty()) {
    onTransformedTofMeasurement(transformed_measurement_vec);
  }
}

void MeasurementClient::onThermalDispatcher(const device::DeviceGroup& group) {
  std::vector<measurement::ThermalMeasurement> measurement_vec;

  group.invokeForEachDeviceOfType<device::HTPA32_Device>([&measurement_vec](device::HTPA32_Device* device) {
    if (!device->getEnable())
      return;
    auto [meas, state] = device->getLatestMeasurement();
    if (state == device::SensorState::SensorOK) {
      measurement_vec.emplace_back(meas);
    }
  });

  if (!measurement_vec.empty()) {
    onThermalMeasurement(measurement_vec);
  }
}

} // namespace manager

} // namespace eduart