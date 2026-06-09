#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   depth_map_matplotlib.py
 @author EduArt Robotik GmbH
 @brief  This example shows the measurement of the first connected ToF sensor in a live matplotlib window.
         It displays two subplots: a 3D scatter plot of the point cloud to the left, and a histogram of the
         sigma (standard deviation) distribution of valid points to the right.
 @date 2025-11-18
"""

import time
import numpy as np
import matplotlib.pyplot as plt

import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SocketCan

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_UsbTingo

# Distance range for axis limits (in meters)
MIN_DIST = 0.0
MAX_DIST = 2.0

# Fixed bin range for sigma histogram (in meters, typically very small)
SIGMA_MAX = 0.01


class DepthMapClient:
  """Client that copies ToF point clouds to a NumPy buffer for visualization."""

  def __init__(self, manager):
    # Buffer for point cloud data: 64 points, 6 columns (x, y, z, raw_distance, sigma, sensor_index)
    self._points_np = np.zeros((64, 6), dtype=np.float64)
    self._got_measurement = False
    self._state = sensorring.ManagerState_Uninitialized
    self._subscriptions = []
    self._subscriptions.append(
      manager.subscribeToStateChanges(self._on_state_change))
    self._subscriptions.append(
      manager.depthSensors().subscribe(self._on_depth_measurement))

  def _on_state_change(self, state):
    self._state = state

  def _on_depth_measurement(self, meas):
    meas.point_cloud.copyTo(self._points_np)
    self._got_measurement = True

  def wait_for_new_measurement(self):
    """Block until the next measurement arrives and return the NumPy point buffer."""
    self._got_measurement = False
    while not self._got_measurement and self._state != sensorring.ManagerState_Shutdown:
      time.sleep(0.001)
    return self._points_np


def main():
  print("=========================================")
  print("Depth map matplotlib sensorring example")
  print("=========================================")
  print()

  params = sensorring.ManagerParams()

  can_interface = sensorring.ComInterfaceID()
  can_interface.type = CAN_INTERFACE_TYPE
  can_interface.name = CAN_INTERFACE_NAME

  usbtingo_interface = sensorring.ComInterfaceID()
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME

  try:
    # Subscribe to the log messages
    log_sub = sensorring.Logger.getInstance().subscribe(
      lambda verbosity, msg:
        print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")
        if verbosity > sensorring.LogVerbosity_Debug else None
    )

    # Create a SensorRing with one depth sensor board via auto-discovery
    factory = sensorring.SensorRingFactory()

    tmf_params = sensorring.TMF8829_Params()
    tmf_params.resolution_mode = sensorring.ResolutionMode_Res16x16
    factory.setDefaultTMF8829Params(tmf_params)

    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams())

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    # Instantiate a DepthMapClient that registers itself with the manager
    client = DepthMapClient(manager)

    # Start the measurements
    manager.startMeasuring()

    # Create matplotlib figure with two subplots: 3D scatter (left) and sigma histogram (right)
    fig = plt.figure(figsize=(12, 5))
    ax_3d = fig.add_subplot(1, 2, 1, projection="3d")
    ax_hist = fig.add_subplot(1, 2, 2)

    # Configure the 3D scatter plot
    d = np.tan(np.deg2rad(22.5)) * MAX_DIST
    ax_3d.set(xlim3d=(-d, d), xlabel='X')
    ax_3d.set(ylim3d=(-d, d), ylabel='Y')
    ax_3d.set(zlim3d=(MIN_DIST, MAX_DIST), zlabel='Z')
    ax_3d.set_aspect('equal')
    scatter = ax_3d.scatter([], [], [])

    # Configure the sigma histogram
    sigma_bins = np.linspace(0, SIGMA_MAX, 64)#121)

    plt.ion()
    plt.tight_layout()
    plt.show(block=False)

    while manager.isMeasuring():
      points = client.wait_for_new_measurement()
      valid = points[:, 3] > 0

      # Update 3D scatter plot
      scatter._offsets3d = (points[valid, 0], points[valid, 1], points[valid, 2])
      ax_3d.relim()
      ax_3d.autoscale_view()

      # Update sigma histogram
      ax_hist.clear()
      ax_hist.set_xlim(0, SIGMA_MAX)
      sigmas = points[valid, 4]
      if len(sigmas) > 0:
        sigmas_clipped = np.clip(sigmas, 0, SIGMA_MAX)
        ax_hist.hist(sigmas_clipped, bins=sigma_bins, edgecolor="black", alpha=0.7)
        mean_sigma = np.mean(sigmas)
        ax_hist.axvline(mean_sigma, color="red", linestyle="--", label=f"mean = {mean_sigma:.5f} m")
        ax_hist.legend()
        ax_hist.set_ylim(0, 64)
      else:
        ax_hist.set_ylim(0, 20)
      ax_hist.set_xlabel("Sigma (m)")
      ax_hist.set_ylabel("Count")
      ax_hist.set_title("Sigma distribution of valid ToF points")

      fig.canvas.draw()
      fig.canvas.flush_events()

    # Stop the measurements
    manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
