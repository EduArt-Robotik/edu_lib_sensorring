#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   depth_map_terminal.py
 @author EduArt Robotik GmbH
 @brief  This example prints a depth map of the first connected ToF Sensor on the command line.
 @date 2025-11-18
"""

import time
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SocketCan

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_UsbTingo

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

  for row in range(meas.resolution_x):
    for col in range(meas.resolution_y):
      idx = row * meas.resolution_y + col
      print(depth_to_color(meas.point_cloud.data[idx].raw_distance, MIN_DIST, MAX_DIST) + "██", end="")
    print("\033[0m")

  print("", end="", flush=True)


class DepthMapClient:
  """Client that prints a colored depth map from the first ToF sensor."""

  def __init__(self, manager, reset_cursor):
    self._init_flag = False
    self._reset_cursor = reset_cursor
    self._subscriptions = []
    self._subscriptions.append(
      manager.subscribeToStateChanges(self._on_state_change))
    self._subscriptions.append(
      manager.depthSensors().subscribe(self._on_depth_measurement))

  def _on_state_change(self, state):
    print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")

  def _on_depth_measurement(self, meas):
    self._init_flag = True
    print_depth_map(meas, self._reset_cursor[0])
    self._reset_cursor[0] = True

  def got_first_measurement(self):
    return self._init_flag


def main():
  print("\33c")
  print("====================================")
  print("Depth map terminal sensorring example")
  print("====================================")
  print()

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

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    # Instantiate a DepthMapClient that registers itself with the manager
    client = DepthMapClient(manager, reset_cursor)

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