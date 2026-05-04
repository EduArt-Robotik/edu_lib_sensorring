#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   using_lambdas.py
 @author EduArt Robotik GmbH
 @brief  This example receives measurements and prints the current measurement rate to the command line.
         It uses the function-based subscription API with plain Python callbacks.
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


class Rate:
  """Rate measurement utility for tracking callback frequency."""

  def __init__(self):
    self._counter = 0
    self._sensor_count = 0
    self._duration = 0.0
    self._init_flag = False
    self._last_measurement = time.time()

  def tick(self, sensor_count=0):
    now = time.time()
    self._duration += now - self._last_measurement
    self._last_measurement = now
    self._init_flag = True
    self._counter += 1
    self._sensor_count = sensor_count

  def get_rate(self):
    if self._init_flag and self._duration > 0:
      rate = self._counter / self._duration
      self._duration = 0.0
      self._counter = 0
      return rate
    return 0.0

  def get_sensor_count(self):
    return self._sensor_count

  def got_first_measurement(self):
    return self._init_flag


def main():
  print("==========================================")
  print("Minimal sensorring example (using lambdas)")
  print("==========================================")
  print()

  params = sensorring.ManagerParams()

  vl53l8cx_rate = Rate()
  htpa32_rate = Rate()

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

    # Create the SensorRing via auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.addInterface(usbtingo_interface)

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    depth_sensor_count = manager.depthSensors().size()
    thermal_sensor_count = manager.thermalSensors().size()

    # Subscribe to the state changes
    state_sub = manager.subscribeToStateChanges(
      lambda state: print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")
    )

    def on_depth_measurement(meas):
      if meas.sensor_index == 0:
        vl53l8cx_rate.tick(depth_sensor_count)

    def on_thermal_measurement(meas):
      if meas.sensor_index == 0:
        htpa32_rate.tick(thermal_sensor_count)

    # Subscribe to all depth sensors for measurement rate tracking
    depth_sub = manager.depthSensors().subscribe(on_depth_measurement)

    # Subscribe to all thermal sensors for measurement rate tracking
    thermal_sub = manager.thermalSensors().subscribe(on_thermal_measurement)

    # Start the measurements
    manager.startMeasuring()

    while not (vl53l8cx_rate.got_first_measurement() or htpa32_rate.got_first_measurement()) and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      print("\nStart printing measurement rate.")
      while manager.isMeasuring():
        print(
          f"\rCurrent measurement rate: {vl53l8cx_rate.get_rate():5.2f} Hz (ToF) from {vl53l8cx_rate.get_sensor_count()} sensors, "
          f"{htpa32_rate.get_rate():5.2f} Hz (Thermal) from {htpa32_rate.get_sensor_count()} sensors",
          end="", flush=True)
        time.sleep(1)

      # Cancel subscriptions before stopping (optional - destruction also cancels)
      state_sub.cancel()
      depth_sub.cancel()
      thermal_sub.cancel()
      log_sub.cancel()

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()