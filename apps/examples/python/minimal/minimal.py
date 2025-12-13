# Copyright (c) 2025 EduArt Robotik GmbH

"""
 @file   minimal.py
 @author EduArt Robotik GmbH
 @brief  This example receives measurements and prints the current measurement rate to the command line.
 @date 2025-11-18
"""

import time
import eduart.sensorring as sensorring


# Sensor setup
SENSOR_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO
SENSOR_INTERFACE_NAME = "0"


# Proxy class that implements the sensorring callbacks to get
# measurements and the log output of the sensorring library
class MeasurementProxy(sensorring.SensorringClient):

  def __init__(self):
    # Initialize base class
    super().__init__()

    self._counter = 0
    self._duration = 0.0
    self._init_flag = False
    self._last_measurement = time.time()


  # Base class callback
  def onRawTofMeasurement(self, measurement_vec):
    self._duration += (time.time() - self._last_measurement)
    self._last_measurement = time.time()
    self._init_flag = True
    self._counter += 1


  # Base class callback
  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print("[" + sensorring.LogVerbosityToString(verbosity) + "] " + msg)


  def getRate(self):
    rate = self._counter / self._duration
    self._duration = 0.0
    self._counter = 0
    return rate


  def gotFirstMeasurement(self):
    return self._init_flag


def main():
  print("==========================")
  print("Minimal sensorring example")
  print("==========================")

  # Create the parameter structure that is used to instantiate the sensorring
  params = sensorring.ManagerParams()
  
  tof = sensorring.TofSensorParams()
  tof.user_idx = 0
  tof.enable = True

  board = sensorring.SensorBoardParams()
  board.tof_params = tof

  bus = sensorring.BusParams()
  bus.type = SENSOR_INTERFACE_TYPE
  bus.interface_name = SENSOR_INTERFACE_NAME
  bus.board_param_vec.append(board)

  ring = sensorring.RingParams()
  ring.bus_param_vec.append(bus)
  ring.timeout_ms = 1000

  params.ring_params = ring
  
  # Instantiate a Measurement proxy
  proxy = MeasurementProxy()

  # Register the proxy with the Logger to get the log output
  sensorring.Logger.getInstance().registerClient(proxy)
  
  try:
    # Instantiate a MeasurementManager with the parameters from above
    manager = sensorring.MeasurementManager(params)

    # Register the proxy with the LogMeasurementManager to get the measurements
    manager.registerClient(proxy)

    # Start the measurements
    manager.startMeasuring()

    while (not proxy.gotFirstMeasurement() and manager.isMeasuring()):
      time.sleep(0.1)

    if(manager.isMeasuring()):
      
      print("\nPrinting measurement rate:")
      while (manager.isMeasuring()):
        print(f"\rCurrent rate: {proxy.getRate():5.2f} Hz", end="", flush=True)
        time.sleep(1)

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print("Caught: ", e)


if __name__ == "__main__":
    main()