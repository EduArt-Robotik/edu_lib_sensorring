#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   thermal_map.py
 @author EduArt Robotik GmbH
 @brief  This example prints a false-color thermal image of the first connected HTPA32 sensor on the command line.
 @date 2025-11-18
"""

import time
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SOCKETCAN

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO


def color_string_command(r, g, b):
  """Return an ANSI true-color escape sequence for the given RGB values."""
  return f"\033[38;2;{r};{g};{b}m"


def print_false_color_image(img, reset_cursor):
  """Print a 32x32 false-color thermal image to the terminal."""
  if reset_cursor:
    print("\033[32F", end="")

  for row in range(32):
    for col in range(32):
      idx = row * 32 + col
      print(color_string_command(img.data[idx][0], img.data[idx][1], img.data[idx][2]) + "██", end="")
    print("\033[0m")

  print("", end="", flush=True)


class ThermalMapClient(sensorring.MeasurementClient, sensorring.LoggerClient):
  """Client that prints a false-color thermal image from the first HTPA32 sensor."""

  def __init__(self, manager):
    sensorring.MeasurementClient.__init__(self)
    sensorring.LoggerClient.__init__(self)
    self._init_flag = False
    self._reset_cursor = False
    self.registerClient(manager)

  def onStateChange(self, state):
    print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")

  def onThermalMeasurement(self, measurement_vec):
    self._init_flag = True
    print_false_color_image(measurement_vec[0].falsecolor_img, self._reset_cursor)
    self._reset_cursor = True

  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")
      self._reset_cursor = False

  def got_first_measurement(self):
    return self._init_flag


def main():
  print("\33c")
  print("==============================")
  print("Thermal map sensorring example")
  print("==============================")
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
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
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
      while manager.isMeasuring():
        time.sleep(1)

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
