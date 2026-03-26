#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   sigma_histogram.py
 @author EduArt Robotik GmbH
 @brief  Live histogram of sigma (std deviation) of valid points from the first ToF sensor.
 @date 2026-01-29
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


class SigmaHistogramClient(sensorring.MeasurementClient, sensorring.LoggerClient):
  """Client that extracts sigma values from ToF point clouds for histogram visualization."""

  def __init__(self, manager):
    sensorring.MeasurementClient.__init__(self)
    sensorring.LoggerClient.__init__(self)
    # Buffer for point cloud data: 64 points max, 6 columns (x, y, z, raw_distance, sigma, user_idx)
    self._points_np = np.zeros((64, 6), dtype=np.float64)
    self._sigma_values = np.array([], dtype=np.float64)
    self._got_measurement = False
    self._state = sensorring.ManagerState_Uninitialized
    self.registerClient(manager)

  def onStateChange(self, state):
    self._state = state

  def onRawTofMeasurement(self, measurement_vec):
    # Copy point cloud data to numpy array to avoid SWIG lifetime issues
    measurement_vec[0].point_cloud.copyTo(self._points_np)

    # Extract valid points: raw_distance > 0 (column 3), sigma is in column 4
    valid_mask = self._points_np[:, 3] > 0.0
    self._sigma_values = self._points_np[valid_mask, 4].copy()
    self._got_measurement = True

  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")

  def wait_for_new_measurement(self):
    """Block until the next measurement arrives and return a copy of the sigma values."""
    self._got_measurement = False
    while not self._got_measurement and self._state != sensorring.ManagerState_Shutdown:
      time.sleep(0.001)
    return self._sigma_values.copy()


def main():
  print("====================================")
  print("Sigma histogram sensorring example")
  print("====================================")
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

    # Instantiate a SigmaHistogramClient that registers itself with the manager
    client = SigmaHistogramClient(manager)

    # Start the measurements
    manager.startMeasuring()

    # Set up the histogram plot
    fig, ax = plt.subplots()
    ax.set_xlabel("Sigma (m)")
    ax.set_ylabel("Count")
    ax.set_title("Sigma of valid ToF points (live)")
    ax.set_ylim(0, 20)
    plt.ion()
    plt.show(block=False)

    # Fixed bin range so the plot stays stable; sigma is in meters, typically small
    sigma_max = 0.003
    bins = np.linspace(0, sigma_max, 121)

    while manager.isMeasuring():
      sigmas = client.wait_for_new_measurement()

      if len(sigmas) > 0:
        sigmas_clipped = np.clip(sigmas, 0, sigma_max)
        ax.clear()
        ax.hist(sigmas_clipped, bins=bins, edgecolor="black", alpha=0.7)
        ax.set_xlabel("Sigma (m)")
        ax.set_ylabel("Count")
        ax.set_title("Sigma of valid ToF points (live)")
        ax.set_ylim(0, 64)
        mean_sigma = np.mean(sigmas)
        ax.axvline(mean_sigma, color="red", linestyle="--", label=f"mean = {mean_sigma:.5f} m")
        ax.legend()
      else:
        ax.clear()
        ax.set_xlabel("Sigma (m)")
        ax.set_ylabel("Count")
        ax.set_title("Sigma of valid ToF points (live) - no valid points")
        ax.set_ylim(0, 20)
        print("No valid points in last frame")

      fig.canvas.draw()
      fig.canvas.flush_events()

    # Stop the measurements
    manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")
  finally:
    plt.ioff()
    plt.show(block=True)


if __name__ == "__main__":
  main()
