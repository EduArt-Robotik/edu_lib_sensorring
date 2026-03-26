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
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SOCKETCAN

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO

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


def print_depth_map(points, reset_cursor):
  """Print an 8x8 colored depth map to the terminal."""
  if reset_cursor:
    print("\033[8F", end="")

  for row in range(8):
    for col in range(8):
      idx = row * 8 + col
      print(depth_to_color(points.data[idx].raw_distance, MIN_DIST, MAX_DIST) + "██", end="")
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

  can_interface = sensorring.ComInterfaceID()
  can_interface.type = CAN_INTERFACE_TYPE
  can_interface.name = CAN_INTERFACE_NAME

  usbtingo_interface = sensorring.ComInterfaceID()
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME

  try:
    # Subscribe to the log messages
    log_sub = sensorring.Logger.getInstance().subscribe(
      lambda verbosity, msg: (
        print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}"),
        reset_cursor.__setitem__(0, False)
      ) if verbosity > sensorring.LogVerbosity_Debug else None
    )

    # Create a SensorRing with one VL53L8CX board via auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.VL53L8CX_Params())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.VL53L8CX_Params())
    sensor_ring = factory.build(sensorring.ValidationMode_Relaxed)

    if sensor_ring is None:
      print("Failed to create SensorRing. Exiting.")
      return

    # Create the MeasurementManager with the SensorRing
    manager = sensorring.MeasurementManager(params, sensor_ring)

    # Subscribe to the state changes
    state_sub = manager.subscribeToStateChanges(
      lambda state: print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")
    )

    # Subscribe to the VL53L8CX device group to get the measurements
    def on_vl53l8cx_measurement(group):
      got_first_measurement[0] = True
      measurement = sensorring.DeviceGroup_getVL53L8CXMeasurement(group, 0)
      print_depth_map(measurement.point_cloud, reset_cursor[0])
      reset_cursor[0] = True

    vl53l8cx_sub = manager.subscribeToDeviceGroup(
      sensorring.DeviceType_VL53L8CX, on_vl53l8cx_measurement
    )

    # Start the measurements
    manager.startMeasuring()

    while not got_first_measurement[0] and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      while manager.isMeasuring():
        time.sleep(1)

      # Cancel subscriptions before stopping (optional - destruction also cancels)
      state_sub.cancel()
      vl53l8cx_sub.cancel()
      log_sub.cancel()

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
