#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   depth_view.py
 @author EduArt Robotik GmbH
 @brief  This example shows the measurement of the first connected ToF Sensor in a 3D plot.
 @date 2025-11-18
"""

import time
import numpy as np
import matplotlib.pyplot as plt

import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SOCKETCAN

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO

# Distance range for axis limits (in meters)
MIN_DIST = 0.0
MAX_DIST = 2.0


class DepthViewClient(sensorring.MeasurementClient, sensorring.LoggerClient):
  """Client that copies ToF point clouds to a NumPy buffer for 3D visualization."""

  def __init__(self, manager):
    sensorring.MeasurementClient.__init__(self)
    sensorring.LoggerClient.__init__(self)
    self._points_np = np.zeros((64, 6), dtype=np.float64)
    self._got_measurement = False
    self._state = sensorring.ManagerState_Uninitialized
    self.registerClient(manager)

  def onStateChange(self, state):
    self._state = state

  def onRawTofMeasurement(self, measurement_vec):
    measurement_vec[0].point_cloud.copyTo(self._points_np)
    self._got_measurement = True

  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")

  def wait_for_new_measurement(self):
    """Block until the next measurement arrives and return the NumPy point buffer."""
    self._got_measurement = False
    while not self._got_measurement and self._state != sensorring.ManagerState_Shutdown:
      time.sleep(0.001)
    return self._points_np


def main():
  print("=============================")
  print("Depth view sensorring example")
  print("=============================")
  print()

  params = sensorring.ManagerParams()

  can_interface = sensorring.ComInterfaceID()
  can_interface.type = CAN_INTERFACE_TYPE
  can_interface.name = CAN_INTERFACE_NAME

  usbtingo_interface = sensorring.ComInterfaceID()
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME

  try:
    # Create a SensorRing with one VL53L8CX board via auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    sensor_ring = factory.build(sensorring.ValidationMode_Relaxed)

    if sensor_ring is None:
      print("Failed to create SensorRing. Exiting.")
      return

    # Create the MeasurementManager with the SensorRing
    manager = sensorring.MeasurementManager(params, sensor_ring)

    # Instantiate a DepthViewClient that registers itself with the manager
    client = DepthViewClient(manager)

    # Start the measurements
    manager.startMeasuring()

    # Create matplotlib 3D plot
    fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
    d = np.tan(np.deg2rad(22.5)) * MAX_DIST
    ax.set(xlim3d=(-d, d), xlabel='X')
    ax.set(ylim3d=(-d, d), ylabel='Y')
    ax.set(zlim3d=(MIN_DIST, MAX_DIST), zlabel='Z')
    ax.set_aspect('equal')

    scatter = ax.scatter([], [], [])
    plt.ion()
    plt.show(block=False)

    while manager.isMeasuring():
      points = client.wait_for_new_measurement()

      idx = points[:, 3] > 0
      scatter._offsets3d = (points[idx, 0], points[idx, 1], points[idx, 2])

      ax.relim()
      ax.autoscale_view()
      fig.canvas.draw()
      fig.canvas.flush_events()

    # Stop the measurements
    manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()