#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   client_interface.py
 @author EduArt Robotik GmbH
 @brief  This example shows how to use the optional MeasurementClient and LoggerClient interfaces.
         A custom client class inherits from both interfaces and overrides their virtual callback methods.
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


class CustomClient(sensorring.MeasurementClient, sensorring.LoggerClient):
  """Client class inheriting from both MeasurementClient and LoggerClient."""

  def __init__(self, manager):
    sensorring.MeasurementClient.__init__(self)
    sensorring.LoggerClient.__init__(self)
    self.vl53l8cx_rate = Rate()
    self.htpa32_rate = Rate()
    self.registerClient(manager)

  def __del__(self):
    self.unregisterClient()

  def onStateChange(self, state):
    print(f"[State] State changed to: {sensorring.ManagerStateToString(state)}")

  def onRawTofMeasurement(self, measurement_vec):
    self.vl53l8cx_rate.tick(len(measurement_vec))

  def onThermalMeasurement(self, measurement_vec):
    self.htpa32_rate.tick(len(measurement_vec))

  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")


def main():
  print("========================================")
  print("Client interface sensorring example")
  print("========================================")
  print()

  params = sensorring.ManagerParams()

  can_interface = sensorring.ComInterfaceID()
  can_interface.type = CAN_INTERFACE_TYPE
  can_interface.name = CAN_INTERFACE_NAME

  usbtingo_interface = sensorring.ComInterfaceID()
  usbtingo_interface.type = USBTINGO_INTERFACE_TYPE
  usbtingo_interface.name = USBTINGO_INTERFACE_NAME

  try:
    # Create SensorRing via factory auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.addInterface(usbtingo_interface)
    sensor_ring = factory.build(sensorring.ValidationMode_Relaxed)

    if sensor_ring is None:
      print("Failed to create SensorRing. Exiting.")
      return

    # Create the MeasurementManager with the SensorRing
    manager = sensorring.MeasurementManager(params, sensor_ring)

    # Instantiate a CustomClient that registers itself with the manager
    client = CustomClient(manager)

    # Start the measurements
    manager.startMeasuring()

    while not (client.vl53l8cx_rate.got_first_measurement() or client.htpa32_rate.got_first_measurement()) and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      print("\nStart printing measurement rate.")
      while manager.isMeasuring():
        print(
          f"\rCurrent measurement rate: {client.vl53l8cx_rate.get_rate():5.2f} Hz (ToF) from {client.vl53l8cx_rate.get_sensor_count()} sensors, "
          f"{client.htpa32_rate.get_rate():5.2f} Hz (Thermal) from {client.htpa32_rate.get_sensor_count()} sensors",
          end="", flush=True)
        time.sleep(1)

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
