// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthSensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for depth-sensing devices (ToF, structured light, etc.).
 * @date   2026-05-08
 */

#pragma once

#include <functional>
#include <vector>

#include "sensorring/device/Sensor.hpp"
#include "sensorring/device/depth/DepthSensorConfig.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Publisher.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class DepthSensor
 * @brief Base class for any depth-sensing device.
 *
 * Inherits from Sensor (which provides Device identity + measurement sync).
 * Users subscribe to depth measurements via subscribe(). The concrete sensor
 * implementation publishes measurements by calling publishMeasurement() from
 * the state machine thread.
 */
class SENSORRING_EXPORT DepthSensor : public Sensor {
public:
  /// Measurement type produced by this sensor category.
  using MeasurementType = measurement::DepthMeasurement;

  /**
   * @brief Construct a DepthSensor with Device params, field-of-view and pixel resolution.
   * @param[in] id        Device identifier.
   * @param[in] interface Communication interface.
   * @param[in] target    Communication endpoint this device listens to.
   * @param[in] enable    Whether the device starts enabled.
   * @param[in] config    Configuration options for the depth sensor.
   * @param[in] fov_x_deg Horizontal field of view in degrees.
   * @param[in] fov_y_deg Vertical field of view in degrees.
   * @param[in] res_x     Horizontal resolution in pixels (columns).
   * @param[in] res_y     Vertical resolution in pixels (rows).
   */
  DepthSensor(DeviceID id, com::ComInterface* interface, com::ComEndpoint target, bool enable, DepthSensorConfig config, double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y);

  /// @brief Virtual destructor.
  virtual ~DepthSensor() = default;

  /**
   * @brief Get the most recent measurement.
   * @return Reference to the latest depth measurement (check header.state for validity).
   */
  const measurement::DepthMeasurement& getLatestMeasurement() const;

  /**
   * @brief Subscribe to depth measurements from this sensor.
   * @param[in] callback Invoked with each new measurement.
   * @return RAII Subscription that auto-cancels on destruction.
   */
  subscription::Subscription subscribe(std::function<void(const measurement::DepthMeasurement&)> callback) { return _depth_publisher.subscribe(std::move(callback)); }

  /**
   * @brief Build a DepthMeasurement from internal state and publish to all subscribers.
   *
   * Called by the state machine after a successful measurement fetch.
   */
  virtual void publishMeasurement();

protected:
  /**
   * @brief Update the sensor's FOV and resolution, and recalculate the lookup tables.
   * @param[in] fov_x_deg New horizontal field of view in degrees.
   * @param[in] fov_y_deg New vertical field of view in degrees.
   * @param[in] res_x     New horizontal resolution in pixels (columns).
   * @param[in] res_y     New vertical resolution in pixels (rows).
   */
  void updateResolution(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y);

  /**
   * @brief Create lookup tables for x and y angles based on FOV and resolution.
   * @param[in] fov_x Horizontal field of view in degrees.
   * @param[in] fov_y Vertical field of view in degrees.
   * @param[in] res_x Horizontal resolution (number of columns).
   * @param[in] res_y Vertical resolution (number of rows).
   * @param[out] lut_x Output vector for horizontal angle lookup table. Output length is res_x.
   * @param[out] lut_y Output vector for vertical angle lookup table. Output length is res_y.
   * @param[out] lut_z Output vector for depth lookup table. Output length is res_x * res_y.
   */
  virtual void createLookupTable(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y, std::vector<double>& lut_x, std::vector<double>& lut_y, std::vector<double>& lut_z);

  /**
   * @brief Calculate the x,y,z coordinates from the raw distance measurements.
   *
   * Uses the internally managed lookup tables (_lut_x/_lut_y), which are
   * updated on construction and via updateResolution().
   * @param[in,out] pcl Point cloud to operate on. Distance must be populated, x,y,z will be calculated and filled in.
   */
  void processRawMeasurement(measurement::PointCloud& pcl);

  DepthSensorConfig _config;
  double _fov_x_deg;
  double _fov_y_deg;
  unsigned int _resolution_x;
  unsigned int _resolution_y;
  std::vector<double> _lut_x;
  std::vector<double> _lut_y;
  std::vector<double> _lut_z;

  measurement::DepthMeasurement _latest_measurement;
  subscription::Publisher<const measurement::DepthMeasurement&> _depth_publisher;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
