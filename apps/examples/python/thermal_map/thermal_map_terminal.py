#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   thermal_map_terminal.py
 @author EduArt Robotik GmbH
 @brief  This example prints a false-color thermal image of the first connected HTPA32 sensor on the command line.
 @date 2025-11-18
"""

import sys
import time
import numpy as np
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SocketCan

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_UsbTingo


def color_string_command(r, g, b):
  """Return an ANSI true-color escape sequence for the given RGB values."""
  return f"\033[38;2;{r};{g};{b}m"


def print_false_color_image(img, reset_cursor):
  """Print a 32x32 false-color thermal image to the terminal."""
  buf = np.zeros((32, 32, 3), dtype=np.uint8)
  img.copyTo(buf)

  parts = []
  if reset_cursor:
    parts.append("\033[32F")

  for row in range(32):
    for col in range(32):
      parts.append(color_string_command(buf[row, col, 0], buf[row, col, 1], buf[row, col, 2]) + "██")
    parts.append("\033[0m\n")

  sys.stdout.write("".join(parts))
  sys.stdout.flush()


class ThermalMapClient(sensorring.LoggerClient):
  """Client that prints a false-color thermal image from the first HTPA32 sensor."""

  def __init__(self, manager):
    sensorring.LoggerClient.__init__(self)
    self._init_flag = False
    self._reset_cursor = False
    self._subscriptions = []
    self._subscriptions.append(
      manager.subscribeToStateChanges(self._on_state_change))
    self._subscriptions.append(
      manager.subscribeToDeviceGroup(sensorring.DeviceType_HTPA32, self._on_htpa32_callback))

  def _on_state_change(self, state):
    print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")

  def _on_htpa32_callback(self, group):
    self._init_flag = True
    measurement = sensorring.DeviceGroup_getHTPA32Measurement(group, 0)
    print_false_color_image(measurement.falsecolor_img, self._reset_cursor)
    self._reset_cursor = True

  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")
      self._reset_cursor = False

  def got_first_measurement(self):
    return self._init_flag


def main():
  print("\33c")
  print("======================================")
  print("Thermal map terminal sensorring example")
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

    # Instantiate a ThermalMapClient that registers itself with the manager
    client = ThermalMapClient(manager)

    # Start the measurements
    manager.startMeasuring()

    while not client.got_first_measurement() and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      # Start calibration of thermal sensors via extra action
      manager.enqueueExtraAction(
        lambda: sensorring.HTPA32_startCalibration(manager, 20)
      )

      while manager.isMeasuring():
        time.sleep(1)

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
