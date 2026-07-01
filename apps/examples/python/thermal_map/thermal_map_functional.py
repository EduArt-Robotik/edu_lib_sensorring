#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   thermal_map_functional.py
 @author EduArt Robotik GmbH
 @brief  This example prints a false-color thermal image of the first connected HTPA32 sensor on the command line.
         It uses the function-based subscription API with plain Python callbacks, matching the
         C++ thermal_map example.
 @date 2025-11-18
"""

import sys
import time
import numpy as np
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"


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


def main():
  print("\33c")
  print("==============================")
  print("Thermal map sensorring example")
  print("==============================")
  print()

  got_first_measurement = [False]
  reset_cursor = [False]

  params = sensorring.ManagerParams()
  params.frequency_thermal_hz = 5.0

  can_interface = sensorring.SocketCanParams()
  can_interface.name = CAN_INTERFACE_NAME

  usbtingo_interface = sensorring.UsbTingoParams()
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME

  try:
    # Subscribe to the log messages
    log_sub = sensorring.Logger.getInstance().subscribe(
      lambda verbosity, msg: (
        print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}"),
        reset_cursor.__setitem__(0, False)
      ) if verbosity > sensorring.LogVerbosity_Debug else None
    )

    # Create a SensorRing with one HTPA32 board via auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.expectDevice(sensorring.ThermalSensorParams())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.expectDevice(sensorring.ThermalSensorParams())

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    # Subscribe to the state changes
    state_sub = manager.subscribeToStateChanges(
      lambda state: print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")
    )

    # Subscribe to thermal sensors to get the measurements
    def on_thermal_measurement(meas):
      got_first_measurement[0] = True
      print_false_color_image(meas.temperatures.toFalseColor(), reset_cursor[0])
      reset_cursor[0] = True

    thermal_sub = manager.thermalSensors().subscribe(on_thermal_measurement)

    # Start the measurements
    manager.startMeasuring()

    while not got_first_measurement[0] and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      # Start calibration directly via the ThermalSensor interface (thread-safe)
      for sensor in manager.thermalSensors():
        sensor.startCalibration(20)

      while manager.isMeasuring():
        time.sleep(1)

      # Cancel subscriptions before stopping (optional - destruction also cancels)
      state_sub.cancel()
      thermal_sub.cancel()
      log_sub.cancel()

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
