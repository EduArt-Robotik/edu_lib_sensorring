#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   depth_map_functional.py
 @author EduArt Robotik GmbH
 @brief  This example prints a depth map of the first connected ToF Sensor on the command line.
         It uses the function-based subscription API with plain Python callbacks, matching the
         C++ depth_map example.
 @date 2025-11-18
"""

import time
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"

# Distance range for color mapping (in meters)
MIN_DIST = 0.0
MAX_DIST = 1.0


def depth_to_color(depth, min_d, max_d):
  """Map a depth value to an ANSI true-color escape sequence."""
  if depth > max_d or depth < 0:
    depth = max_d
  elif depth < min_d:
    depth = min_d

  d = (depth - min_d) / (max_d - min_d)

  # Color gradient: red (near) -> yellow -> green -> blue (far)
  r = int(255 * (1 - d))
  g = int(255 * (1 - abs(0.5 - d) * 2))
  b = int(255 * d)

  return f"\033[38;2;{r};{g};{b}m"


def print_depth_map(meas, reset_cursor):
  """Print a colored depth map to the terminal."""
  if reset_cursor:
    print(f"\033[{meas.resolution_y}F", end="")

  for row in range(meas.resolution_y):
    for col in range(meas.resolution_x):
      idx = row * meas.resolution_x + col
      print(depth_to_color(meas.point_cloud.data[idx].point.z(), MIN_DIST, MAX_DIST) + "██", end="")
    print("\033[0m")

  print("", end="", flush=True)


def main():
  print("\33c")
  print("============================")
  print("Depth map sensorring example")
  print("============================")
  print()

  got_first_measurement = [False]
  reset_cursor = [False]

  params = sensorring.ManagerParams()

  can_interface = sensorring.SocketCanParams(CAN_INTERFACE_NAME)
  usbtingo_interface = sensorring.UsbTingoParams(USBTINGO_INTERFACE_NAME)

  try:
    # Subscribe to the log messages
    log_sub = sensorring.Logger.getInstance().subscribe(
      lambda verbosity, msg: (
        print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}"),
        reset_cursor.__setitem__(0, False)
      ) if verbosity > sensorring.LogVerbosity_Debug else None
    )

    # Create a SensorRing with one depth sensor board via auto-discovery
    factory = sensorring.SensorRingFactory()

    tmf_params = sensorring.TMF8829_Params()
    tmf_params.resolution_mode = sensorring.ResolutionMode_Res16x16
    factory.setDefaultDeviceParams(tmf_params)

    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.expectDevice(sensorring.DepthSensorParams())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams())
    factory.expectDevice(sensorring.DepthSensorParams())

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    # Subscribe to the state changes
    state_sub = manager.subscribeToStateChanges(
      lambda state: print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")
    )

    # Subscribe to depth sensors to get the measurements
    def on_depth_measurement(meas):
      got_first_measurement[0] = True
      print_depth_map(meas, reset_cursor[0])
      reset_cursor[0] = True

    depth_sub = manager.depthSensors().subscribe(on_depth_measurement)

    # Start the measurements
    manager.startMeasuring()

    while not got_first_measurement[0] and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      while manager.isMeasuring():
        time.sleep(1)

      # Cancel subscriptions before stopping (optional - destruction also cancels)
      state_sub.cancel()
      depth_sub.cancel()
      log_sub.cancel()

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
