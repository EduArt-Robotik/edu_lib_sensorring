// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   VL53L8CX_Device.hpp
 * @author EduArt Robotik GmbH
 * @brief  Hardware abstraction for the VL53L8CX Time-of-Flight sensor device
 * @date   2025-02-11
 */

#pragma once

#include <memory>
#include <vector>

#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace sensorring {

namespace device {

class VL53L8CX_DeviceImpl;

/**
 * @class VL53L8CX_Device
 * @brief  Device wrapper for a VL53L8CX Time-of-Flight sensor on the sensorring bus.
 */
class SENSORRING_EXPORT VL53L8CX_Device : public DepthSensor {
public:
  /**
   * @brief Construct a new VL53L8CX device instance.
   * @param[in] params    Sensor configuration parameters.
   * @param[in] interface Communication interface used to talk to the device.
   * @param[in] idx       Index of the sensor on the bus.
   */
  VL53L8CX_Device(VL53L8CX_Params params, com::ComInterfaceID interface, unsigned int idx);
  /// Destructor
  ~VL53L8CX_Device();

  /**
   * @brief Get the sensor parameters used to configure this device.
   * @return Reference to the internal VL53L8CX parameter struct.
   */
  const VL53L8CX_Params& getParams() const override;

protected:
  bool sendMeasurementRequest(std::uint8_t sequence_number) override;
  bool sendMeasurementTransmissionRequest() override;

private:
  /**
   * @brief Communication callback invoked by the bus interface.
   * @param[in] source Endpoint that delivered the data.
   * @param[in] data   Raw payload received from the device.
   */
  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data) override;

  friend class VL53L8CX_DeviceImpl;
  std::unique_ptr<VL53L8CX_DeviceImpl> _impl;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
