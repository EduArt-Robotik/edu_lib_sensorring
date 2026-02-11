#include "VL53L8CX_Device.hpp"

#include <algorithm>
#include <cstring>
#include <thread>
#include <unordered_map>

#include "interface/can/canprotocol.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {

namespace device {

VL53L8CX_Device::VL53L8CX_Device(VL53L8CX_Params params, com::ComInterface* interface, unsigned int idx)
    : BaseDevice(DeviceID({ DeviceType::VL53L8CX, "tof", idx }), interface, com::ComEndpoint("tof" + std::to_string(idx) + "_data"), params.enable)
    , _params(params) {
  _rx_buffer_offset = 0;
  _interface->addTofSensorEndpoint(idx);
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
}

VL53L8CX_Device::~VL53L8CX_Device() {
}

const VL53L8CX_Params& VL53L8CX_Device::getParams() const {
  return _params;
}

std::pair<const measurement::TofMeasurement&, SensorState> VL53L8CX_Device::getLatestRawMeasurement() const {
  return { _latest_raw_measurement, _error };
}

std::pair<const measurement::TofMeasurement&, SensorState> VL53L8CX_Device::getLatestTransformedMeasurement() const {
  return { _latest_transformed_measurement, _error };
}

void VL53L8CX_Device::onResetSensorState() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset = 0;
}

void VL53L8CX_Device::onClearDataFlag() {
  std::fill(std::begin(_rx_buffer), std::end(_rx_buffer), 0);
  _rx_buffer_offset = 0;
}

void VL53L8CX_Device::comCallback([[maybe_unused]] const com::ComEndpoint source, const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(_state_mutex);
  std::size_t msg_size = data.size();

  // point data msg
  if (msg_size == 48) {
    if ((_rx_buffer_offset + msg_size) <= (int)sizeof(_rx_buffer)) {
      std::copy_n(data.begin(), msg_size, (uint8_t*)&_rx_buffer + _rx_buffer_offset);
      _rx_buffer_offset += msg_size;
      _new_data_available_flag.store(false, std::memory_order_release);

      // got all 4 raw data messages
      if (_rx_buffer_offset >= sizeof(_rx_buffer)) {
        _new_data_in_buffer_flag.store(true, std::memory_order_release);
        _data_condition.notify_all();
      }
    } else {
      _error = SensorState::ReceiveError;
    }

    // transmission complete message
  } else if (msg_size == 2) {
    if (_new_data_in_buffer_flag.load(std::memory_order_acquire)) {
      _latest_raw_measurement         = processMeasurement(data[1], _rx_buffer, vl53l8::TOF_RESOLUTION);
      _latest_transformed_measurement = transformTofMeasurements(_latest_raw_measurement, _rot_m, _translation);
      _new_data_in_buffer_flag.store(false, std::memory_order_release);
      _new_measurement_ready_flag.store(true, std::memory_order_release);
      _data_condition.notify_all();
    }

    // data available message
  } else if (msg_size == 1) {
    _new_data_available_flag.store(true, std::memory_order_release);
    _data_condition.notify_all();
  }
}

measurement::TofMeasurement VL53L8CX_Device::processMeasurement(int frame_id, uint8_t* data, int len) const {
  measurement::TofMeasurement result;
  result.point_cloud.data.reserve(vl53l8::TOF_RESOLUTION);
  result.frame_id = frame_id;

  uint16_t distance_raw = 0;
  uint16_t sigma_raw    = 0;

  for (int i = 0; i < len; i++) {
    distance_raw = (*((uint32_t*)(data + i * 3)) >> 10) & 0x3FFF; // 14 bit
    sigma_raw    = (*((uint32_t*)(data + i * 3)) >> 0) & 0x03FF;  // 10 bit

    math::Vector3 point   = { 0, 0, 0 };
    double point_distance = -1;
    double point_sigma    = -1;

    if (distance_raw != 0) {
      point_distance = (double)distance_raw / 4.0F / 1000.0F; // Factor 4 for fixed point conversion, Factor 1000 from mm to m
      point_sigma    = (double)sigma_raw / 128.0 / 1000.0F;   // Factor 128 for fixed point conversion, Factor 1000 from mm to m

      point.x() = point_distance * vl53l8::lut_tan_x[i];
      point.y() = point_distance * vl53l8::lut_tan_y[i];
      point.z() = point_distance;
    }

    result.point_cloud.data.push_back(measurement::PointData({ point, point_distance, point_sigma, _params.user_idx }));
  }

  result.point_cloud.data.shrink_to_fit();
  return result;
}

std::future<bool> VL53L8CX_Device::requestTofMeasurementAsync(std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [this, timeout]() {
    if (!getEnable())
      return false;

    // Clear the flag so that we really wait for a *new* data-available indication
    _new_data_available_flag.store(false, std::memory_order_release);

    static std::atomic<std::uint8_t> request_count{ 0 };
    std::uint8_t count          = request_count.fetch_add(1, std::memory_order_relaxed);
    unsigned int active_sensors = (1u << static_cast<unsigned int>(getIdx()));
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { count, sensor_select_high, sensor_select_low };
    _interface->send(com::ComEndpoint("tof_request"), tx_buf);

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::unique_lock<std::mutex> lock(_state_mutex);
    const bool signaled = _data_condition.wait_until(lock, deadline, [this]() {
      return newDataAvailable();
    });
    return signaled && newDataAvailable();
  });
}

std::future<bool> VL53L8CX_Device::fetchTofMeasurementAsync(std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [this, timeout]() {
    if (!getEnable())
      return false;

    clearDataFlag();
    unsigned int active_sensors = (1u << static_cast<unsigned int>(getIdx()));
    uint8_t sensor_select_high  = (uint8_t)((active_sensors >> 8) & 0xFF);
    uint8_t sensor_select_low   = (uint8_t)((active_sensors >> 0) & 0xFF);
    std::vector<uint8_t> tx_buf = { sensor_select_high, sensor_select_low };
    _interface->send(com::ComEndpoint("tof_request"), tx_buf);

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::unique_lock<std::mutex> lock(_state_mutex);
    const bool signaled = _data_condition.wait_until(lock, deadline, [this]() {
      return gotNewData();
    });
    return signaled && gotNewData();
  });
}

std::future<bool> VL53L8CX_Device::requestTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    // Group enabled devices by their communication interface, clear flags and
    // build per-interface masks in a single pass.
    struct InterfaceGroup {
      std::vector<VL53L8CX_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);
        // Clear flag so that we really wait for *new* data-available indications.
        dev->_new_data_available_flag.store(false, std::memory_order_release);
        group.active_sensors |= (1u << static_cast<unsigned int>(dev->getIdx()));
      }
    }

    if (groups.empty()) {
      return false;
    }

    static std::atomic<std::uint8_t> request_count{ 0 };
    std::uint8_t count = request_count.fetch_add(1, std::memory_order_relaxed);

    // Send one request per interface.
    for (auto& [iface, group] : groups) {
      if (group.devices.empty()) {
        continue;
      }
      unsigned int active_sensors = group.active_sensors;
      uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
      uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
      std::vector<uint8_t> tx_buf{ count, sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint("tof_request"), tx_buf);
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;

    // Wait until all enabled devices (across all interfaces) report new data available.
    for (auto& [iface, group] : groups) {
      for (auto* dev : group.devices) {
        std::unique_lock<std::mutex> lock(dev->_state_mutex);
        const bool signaled = dev->_data_condition.wait_until(lock, deadline, [dev]() {
          return dev->newDataAvailable();
        });
        if (!(signaled && dev->newDataAvailable())) {
          return false;
        }
      }
    }

    return true;
  });
}

std::future<bool> VL53L8CX_Device::fetchTofMeasurementAsync(const std::vector<VL53L8CX_Device*>& devices, std::chrono::milliseconds timeout) {
  return std::async(std::launch::async, [devices, timeout]() {
    // Group enabled devices by their communication interface, clear flags and
    // build per-interface masks in a single pass.
    struct InterfaceGroup {
      std::vector<VL53L8CX_Device*> devices;
      unsigned int active_sensors = 0;
    };
    std::unordered_map<com::ComInterface*, InterfaceGroup> groups;

    for (auto* dev : devices) {
      if (dev != nullptr && dev->getEnable()) {
        auto* iface = dev->_interface;
        auto& group = groups[iface];
        group.devices.push_back(dev);
        // Clear data flags before issuing the shared fetch.
        dev->clearDataFlag();
        group.active_sensors |= (1u << static_cast<unsigned int>(dev->getIdx()));
      }
    }

    if (groups.empty()) {
      return false;
    }

    // Send one fetch per interface.
    for (auto& [iface, group] : groups) {
      if (group.devices.empty()) {
        continue;
      }
      unsigned int active_sensors = group.active_sensors;
      uint8_t sensor_select_high  = static_cast<uint8_t>((active_sensors >> 8) & 0xFF);
      uint8_t sensor_select_low   = static_cast<uint8_t>((active_sensors >> 0) & 0xFF);
      std::vector<uint8_t> tx_buf{ sensor_select_high, sensor_select_low };
      iface->send(com::ComEndpoint("tof_request"), tx_buf);
    }

    const auto deadline = std::chrono::steady_clock::now() + timeout;

    // Wait until all enabled devices (across all interfaces) have produced a new measurement.
    for (auto& [iface, group] : groups) {
      for (auto* dev : group.devices) {
        std::unique_lock<std::mutex> lock(dev->_state_mutex);
        const bool signaled = dev->_data_condition.wait_until(lock, deadline, [dev]() {
          return dev->gotNewData();
        });
        if (!(signaled && dev->gotNewData())) {
          return false;
        }
      }
    }

    return true;
  });
}

measurement::TofMeasurement VL53L8CX_Device::transformTofMeasurements(const measurement::TofMeasurement& measurement, const math::Matrix3 rotation, const math::Vector3 translation) {

  auto transformed_measurement = measurement;

  for (unsigned int i = 0; i < transformed_measurement.point_cloud.data.size(); i++) {
    transformed_measurement.point_cloud.data[i].point = (rotation * measurement.point_cloud.data[i].point) + translation;
  }

  return transformed_measurement;
}

} // namespace device

} // namespace eduart
