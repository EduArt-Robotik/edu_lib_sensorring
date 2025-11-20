# Copyright (c) 2025 EduArt Robotik GmbH

"""
 @file   depth_map.py
 @author EduArt Robotik GmbH
 @brief  This example prints a depth map of the first connected ToF Sensor on the command line.
 @date 2025-11-18
"""

import time
import eduart.sensorring as sensorring


# Sensor setup
SENSOR_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO
SENSOR_INTERFACE_NAME = "0x1731a1f1"


# Threshold values for color mapping
MIN_DIST = 0.0
MAX_DIST = 1.0


# Proxy class that implements the sensorring callbacks to get
# measurements and the log output of the sensorring library
class MeasurementProxy(sensorring.SensorringClient):

  def __init__(self):
    # Initialize base class
    super().__init__()

    # class members
    self._init_flag = False
    

  # Base class callback
  def onRawTofMeasurement(self, measurement_vec):
    self._init_flag = True
    self.printDepthMap(measurement_vec[0].point_cloud)


  # Base class callback
  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print("[" + sensorring.LogVerbosityToString(verbosity) + "] " + msg)
      print("\033[s", end="")


  def gotFirstMeasurement(self):
    return self._init_flag
  

  def depthToColor(self, depth, min, max):
    if depth > max or depth < 0:
      depth = max
    elif depth < min:
      depth = min

    d = (depth - min) / (max - min)

    # Map t to a color gradient: red (near) → yellow → green → blue (far)
    r = int(255 * (1 - d))
    g = int(255 * (1 - abs(0.5 - d) * 2))
    b = int(255 * d)

    return "\033[38;2;" + str(r) + ";" + str(g) + ";" + str(b) + "m"

  def printDepthMap(self, points):
    print("\033[u")
    for row in range(0, 8):
      for col in range(0, 8):
        idx = row * 8 + col
        print(self.depthToColor(points[idx].raw_distance, MIN_DIST, MAX_DIST) + "██", end="")
      
      print("\033[0m")
    print("", end="", flush=True)  


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
        
    while (manager.isMeasuring()):
      time.sleep(1)

    # Stop the measurements
    manager.stopMeasuring()
    
  except Exception as e:
    print("Caught: ", e)

if __name__ == "__main__":
    main()