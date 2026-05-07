// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   DepthSensor.hpp
 * @author EduArt Robotik GmbH
 * @brief  Abstract interface for depth-sensing devices (ToF, structured light, etc.).
 */

#pragma once

#include <functional>
#include <vector>

#include "sensorring/measurement/DepthMeasurement.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/subscription/Publisher.hpp"
#include "sensorring/subscription/Subscription.hpp"

namespace eduart {

namespace sensorring {

namespace device {

/**
 * @class DepthSensor
 * @brief Public interface for any depth-sensing device.
 *
 * Users subscribe to depth measurements via subscribe(). The concrete sensor
 * implementation publishes measurements by calling publishMeasurement() from
 * the state machine thread.
 */
class SENSORRING_EXPORT DepthSensor {
public:
  /// Measurement type produced by this sensor category.
  using MeasurementType = measurement::DepthMeasurement;

  /// Constructor
  DepthSensor(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y);

  /// Destructor
  virtual ~DepthSensor() = default;

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
  virtual void publishMeasurement() = 0;

protected:
  /**
   * @brief Create lookup tables for x and y angles based on FOV and resolution.
   * @param fov_x Horizontal field of view in degrees.
   * @param fov_y Vertical field of view in degrees.
   * @param res_x Horizontal resolution (number of columns).
   * @param res_y Vertical resolution (number of rows).
   * @param lut_x Output vector for horizontal angle lookup table.
   * @param lut_y Output vector for vertical angle lookup table.
   */
  virtual void createLookupTable(double fov_x_deg, double fov_y_deg, unsigned int res_x, unsigned int res_y, std::vector<double>& lut_x, std::vector<double>& lut_y);

  /**
   * @brief Transform a raw depth measurement into a point cloud.
   * @param lut_x Horizontal angle lookup table.
   * @param lut_y Vertical angle lookup table.
   * @param meas Raw depth measurement vector.
   * @param pcl Output point cloud.
   */
  void transformMeasurementToPointCloud(const std::vector<double>& lut_x, const std::vector<double>& lut_y, const std::vector<double>& meas, measurement::PointCloud& pcl);

  double _fov_x_deg;
  double _fov_y_deg;
  unsigned int _resolution_x;
  unsigned int _resolution_y;
  std::vector<double> _lut_x;
  std::vector<double> _lut_y;

  subscription::Publisher<const measurement::DepthMeasurement&> _depth_publisher;
};

} // namespace device

} // namespace sensorring

} // namespace eduart
