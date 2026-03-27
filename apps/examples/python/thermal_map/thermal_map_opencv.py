#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   thermal_map_opencv.py
 @author EduArt Robotik GmbH
 @brief  This example shows the thermal image of the first connected HTPA32 sensor in an OpenCV window.
 @date 2026-03-26
"""

import time
import numpy as np
import cv2

import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SocketCan

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_UsbTingo

# Display scale factor (32x32 is tiny, scale up for visibility)
SCALE_FACTOR = 16


class ThermalViewClient(sensorring.LoggerClient):
  """Client that copies HTPA32 false-color images to a NumPy buffer for OpenCV display."""

  def __init__(self, manager):
    sensorring.LoggerClient.__init__(self)
    self._img_np = np.zeros((32, 32, 3), dtype=np.uint8)
    self._got_measurement = False
    self._state = sensorring.ManagerState_Uninitialized
    self._subscriptions = []
    self._subscriptions.append(
      manager.subscribeToStateChanges(self._on_state_change))
    self._subscriptions.append(
      manager.subscribeToDeviceGroup(sensorring.DeviceType_HTPA32, self._on_htpa32_callback))

  def _on_state_change(self, state):
    self._state = state

  def _on_htpa32_callback(self, group):
    measurement = sensorring.DeviceGroup_getHTPA32Measurement(group, 0)
    measurement.falsecolor_img.copyTo(self._img_np)
    self._got_measurement = True

  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")

  def wait_for_new_measurement(self):
    """Block until the next measurement arrives and return the NumPy image buffer."""
    self._got_measurement = False
    while not self._got_measurement and self._state != sensorring.ManagerState_Shutdown:
      time.sleep(0.001)
    return self._img_np


def main():
  print("======================================")
  print("Thermal map OpenCV sensorring example")
  print("======================================")
  print()

  params = sensorring.ManagerParams()
  params.frequency_thermal_hz = 5.0

  can_interface = sensorring.ComInterfaceID()
  can_interface.type = CAN_INTERFACE_TYPE
  can_interface.name = CAN_INTERFACE_NAME

  usbtingo_interface = sensorring.ComInterfaceID()
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME

  try:
    # Create a SensorRing with one HTPA32 board via auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.HTPA32_Params())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.HTPA32_Params())
    sensor_ring = factory.build(sensorring.ValidationMode_Relaxed)

    if sensor_ring is None:
      print("Failed to create SensorRing. Exiting.")
      return

    # Create the MeasurementManager with the SensorRing
    manager = sensorring.MeasurementManager(params, sensor_ring)

    # Instantiate a ThermalViewClient that registers itself with the manager
    client = ThermalViewClient(manager)

    # Start the measurements
    manager.startMeasuring()

    # Wait for first measurement before starting calibration
    while not client._got_measurement and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      # Start calibration of thermal sensors via extra action
      manager.enqueueExtraAction(
        lambda: sensorring.HTPA32_startCalibration(manager, 20)
      )

    window_name = "Thermal View"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name, 32 * SCALE_FACTOR, 32 * SCALE_FACTOR)

    while manager.isMeasuring():
      img = client.wait_for_new_measurement()

      # Convert RGB to BGR for OpenCV display
      img_bgr = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)

      # Scale up with nearest-neighbor interpolation for a crisp pixel look
      img_scaled = cv2.resize(img_bgr, (32 * SCALE_FACTOR, 32 * SCALE_FACTOR), interpolation=cv2.INTER_NEAREST)

      cv2.imshow(window_name, img_scaled)

      # Exit on 'q' or ESC key
      key = cv2.waitKey(1) & 0xFF
      if key == ord('q') or key == 27:
        break

    # Stop the measurements
    manager.stopMeasuring()
    cv2.destroyAllWindows()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
