"""
Depth Map Python3 example.

This example prints a depth map of the first connected ToF Sensor on the command line.

2025-11-18
"""

import time
import eduart.sensorring as sensorring


# Sensor setup
SENSOR_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO
SENSOR_INTERFACE_NAME = "0x1731a1f1"


# Threshold values for color mapping
MIN_DIST = 0.0
MAX_DIST = 1.0


class MeasurementProxy(sensorring.SensorringClient):

  def __init__(self):
    # Initialize base class
    super().__init__()

    # class members
    self._counter = 0
    self._init_flag = False
    self._last_query = time.time()
    

  # Base class callback
  def onRawTofMeasurement(self, measurement_vec):
    self._init_flag = True
    self._counter += 1
    self.printDepthMap(measurement_vec[0].point_cloud)


  # Base class callback
  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print("[" + sensorring.LogVerbosityToString(verbosity) + "] " + msg)
      print("\033[s", end="")


  def getRate(self):
    now = time.time()
    rate = self._counter / (now - self._last_query)

    self._last_query = now
    self._counter = 0
    return rate


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
  
  proxy = MeasurementProxy()
  sensorring.Logger.getInstance().registerClient(proxy)

  try:
    manager = sensorring.MeasurementManager(params)
    manager.registerClient(proxy)
    manager.startMeasuring()
        
    while (manager.isMeasuring()):
      time.sleep(1)

    manager.stopMeasuring()
    
  except Exception as e:
    print("Caught: ", e)

if __name__ == "__main__":
    main()