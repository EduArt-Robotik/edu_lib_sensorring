#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   using_proxy_class.py
 @author EduArt Robotik GmbH
 @brief  This example shows how to use the SensorRing with a proxy class for object-oriented measurement handling.
         Instead of using lambdas or the client interface, a custom proxy class binds its own member methods
         as subscription callbacks.
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


class CustomProxy:
  """Proxy class that subscribes its own member methods as callbacks."""

  def __init__(self, manager):
    self.vl53l8cx_rate = Rate()
    self.htpa32_rate = Rate()
    self._subscriptions = []

    # Subscribe member methods using bound methods as callbacks
    self._subscriptions.append(
      manager.subscribeToStateChanges(self.on_manager_state_change))
    self._subscriptions.append(
      manager.depthSensors().subscribe(self.on_depth_measurement))
    self._subscriptions.append(
      manager.thermalSensors().subscribe(self.on_thermal_measurement))

  def cancel_all(self):
    """Cancel all subscriptions."""
    for sub in self._subscriptions:
      sub.cancel()
    self._subscriptions.clear()

  def on_manager_state_change(self, state):
    print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")

  def on_depth_measurement(self, meas):
    self.vl53l8cx_rate.tick(1)

  def on_thermal_measurement(self, meas):
    self.htpa32_rate.tick(1)


def main():
  print("===============================================")
  print("Minimal sensorring example (using proxy class)")
  print("===============================================")
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

    # Create SensorRing via factory auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.addInterface(usbtingo_interface)

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    # Instantiate a Measurement proxy
    proxy = CustomProxy(manager)

    # Start the measurements
    manager.startMeasuring()

    while not (proxy.vl53l8cx_rate.got_first_measurement() or proxy.htpa32_rate.got_first_measurement()) and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      print("\nStart printing measurement rate.")
      while manager.isMeasuring():
        print(
          f"\rCurrent measurement rate: {proxy.vl53l8cx_rate.get_rate():5.2f} Hz (ToF) from {proxy.vl53l8cx_rate.get_sensor_count()} sensors, "
          f"{proxy.htpa32_rate.get_rate():5.2f} Hz (Thermal) from {proxy.htpa32_rate.get_sensor_count()} sensors",
          end="", flush=True)
        time.sleep(1)

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
