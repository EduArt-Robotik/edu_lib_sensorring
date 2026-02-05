// Copyright (c) 2025 EduArt Robotik GmbH

/**
 * @file   SensorRing.hpp
 * @author EduArt Robotik GmbH
 * @brief  Top-level container managing multiple sensor buses
 * @date   2025-02-06
 */

#pragma once

#include <memory>
#include <vector>

#include "sensorring/Parameter.hpp"

#include "SensorBus.hpp"


namespace eduart {

namespace ring {

/**
 * @class SensorRing
 * @brief Top-level container managing multiple sensor buses and coordinating operations across them.
 */
class SensorRing {

public:
  /**
   * Constructor
   * @param[in] params Configuration parameters for the sensor ring
   * @param[in] bus_vec Vector of sensor buses to manage
   */
  SensorRing(RingParams params, std::vector<std::unique_ptr<bus::SensorBus> > bus_vec);
  /// Destructor
  ~SensorRing();

  /**
   * @brief Get all sensor bus interfaces managed by this ring.
   * @return Vector of const pointers to all sensor buses.
   */
  std::vector<const bus::SensorBus*> getInterfaces() const;

  /**
   * @brief Enable or disable bit rate switching on CAN bus interfaces.
   * @param[in] brs_enable Enable flag
   */
  void setBrs(bool brs_enable);

  /**
   * @brief Synchronize light animations across all sensor buses.
   */
  void syncLight();

  /**
   * @brief Set light mode and color for all sensor buses.
   * @param[in] mode Light mode to set
   * @param[in] red Red color value
   * @param[in] green Green color value
   * @param[in] blue Blue color value
   */
  void setLight(light::LightMode mode, std::uint8_t red, std::uint8_t green, std::uint8_t blue);

  /**
   * @brief Reset all devices on all sensor buses.
   */
  void resetDevices();

  /**
   * @brief Reset sensor state on all sensor buses.
   */
  void resetSensorState();

  /**
   * @brief Enumerate devices on all sensor buses.
   * @return true if enumeration succeeded on all buses
   */
  bool enumerateDevices();

  /**
   * @brief Request EEPROM data from all sensor buses.
   * @return true if request succeeded on all buses
   */
  bool getEEPROM();

  /**
   * @brief Request Time-of-Flight measurements from all enabled sensors.
   */
  void requestTofMeasurement();

  /**
   * @brief Fetch Time-of-Flight measurement data from all enabled sensors.
   */
  void fetchTofMeasurement();

  /**
   * @brief Request thermal measurements from all enabled sensors.
   */
  void requestThermalMeasurement();

  /**
   * @brief Fetch thermal measurement data from all enabled sensors.
   */
  void fetchThermalMeasurement();

  /**
   * @brief Stop thermal calibration on all sensor buses.
   * @return true if stop succeeded on all buses
   */
  bool stopThermalCalibration();

  /**
   * @brief Start thermal calibration on all sensor buses.
   * @param[in] window Number of thermal frames used for averaging
   * @return true if start succeeded on all buses
   */
  bool startThermalCalibration(std::size_t window);

  /**
   * @brief Wait until all Time-of-Flight measurements are ready.
   * @return true if all measurements are ready
   */
  bool waitForAllTofMeasurementsReady() const;

  /**
   * @brief Wait until all Time-of-Flight data transmissions are complete.
   * @return true if all transmissions are complete
   */
  bool waitForAllTofDataTransmissionsComplete() const;

  /**
   * @brief Wait until all thermal measurements are ready.
   * @return true if all measurements are ready
   */
  bool waitForAllThermalMeasurementsReady() const;

  /**
   * @brief Wait until all thermal data transmissions are complete.
   * @return true if all transmissions are complete
   */
  bool waitForAllThermalDataTransmissionsComplete() const;

  /**
   * @brief Create a SensorRing instance from configuration parameters.
   * @param[in] params Configuration parameters for the sensor ring
   * @return Unique pointer to the created SensorRing instance
   */
  static std::unique_ptr<SensorRing> create(RingParams params);
private:
  const RingParams _params;
  std::vector<std::unique_ptr<bus::SensorBus> > _bus_vec;
};

} // namespace ring

} // namespace eduart