"""
Depth View Python3 example.

This example shows the measurement of the first connected ToF Sensor in a 3D plot.

2025-11-18
"""

import time
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d

import eduart.sensorring as sensorring


# Sensor setup
SENSOR_INTERFACE_TYPE = sensorring.InterfaceType_USBTINGO
SENSOR_INTERFACE_NAME = "0x1731a1f1"


# Threshold values for color mapping
MIN_DIST = 0.0
MAX_DIST = 2.0


# Proxy class that implements the sensorring callbacks to get
# measurements and the log output of the sensorring library
class MeasurementProxy(sensorring.SensorringClient):

  def __init__(self):
    # Initialize base class
    super().__init__()

    # class members
    self._points_np = np.zeros((64, 6), dtype=np.float64)
    self._got_measurement = False
    self._state = sensorring.ManagerState_Uninitialized


  # Base class callback
  def onRawTofMeasurement(self, measurement_vec):
    sensorring.copyPointCloudTo(measurement_vec[0].point_cloud, self._points_np)
    self._got_measurement = True
  

  # Base class callback
  def onOutputLog(self, verbosity, msg):
    if verbosity > sensorring.LogVerbosity_Debug:
      print("[" + sensorring.LogVerbosityToString(verbosity) + "] " + msg)
      print("\033[s", end="")


  # Base class callback
  def onStateChange(self, state):
    self._state = state
  

  def waitForNewMeasurement(self):
    self._got_measurement = False
    while(not self._got_measurement and self._state != sensorring.ManagerState_Shutdown):
      pass
    return self._points_np


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

    # Create matplotlib plot
    fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
    d = np.tan(np.deg2rad(22.5)) * MAX_DIST
    ax.set(xlim3d=(-d, d), xlabel='X')
    ax.set(ylim3d=(-d, d), ylabel='Y')
    ax.set(zlim3d=(MIN_DIST, MAX_DIST), zlabel='Z')
    ax.set_aspect('equal')

    scatter = ax.scatter([], [], [])
    plt.ion()
    plt.show(block=False)

    while (manager.isMeasuring()):
      points = proxy.waitForNewMeasurement()

      idx = points[:,3] > 0
      scatter._offsets3d = (points[idx, 0], points[idx, 1], points[idx, 2])

      ax.relim()
      ax.autoscale_view()
      fig.canvas.draw()
      fig.canvas.flush_events()

    # Stop the measurements
    manager.stopMeasuring()

  except Exception as e:
    print("Caught: ", e)


if __name__ == "__main__":
    main()