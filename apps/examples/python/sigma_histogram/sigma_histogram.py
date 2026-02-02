# Copyright (c) 2025 EduArt Robotik GmbH

"""
 @file   sigma_histogram.py
 @author EduArt Robotik GmbH
 @brief  Live histogram of sigma (std deviation) of valid points from the first ToF sensor.
 @date 2026-01-29
"""

import time
import numpy as np
import matplotlib.pyplot as plt

import eduart.sensorring as sensorring


# Sensor setup
SENSOR_INTERFACE_TYPE = sensorring.InterfaceType_SOCKETCAN
SENSOR_INTERFACE_NAME = "can0"


# Proxy class that implements the sensorring callbacks to get
# measurements and the log output of the sensorring library
class MeasurementProxy(sensorring.SensorringClient):

    def __init__(self):
        super().__init__()
        # Buffer for point cloud data: 64 points max, 6 columns (x, y, z, raw_distance, sigma, user_idx)
        self._points_np = np.zeros((64, 6), dtype=np.float64)
        self._sigma_values = np.array([], dtype=np.float64)
        self._got_measurement = False
        self._state = sensorring.ManagerState_Uninitialized

    def onRawTofMeasurement(self, measurement_vec):
        # Copy point cloud data to numpy array first to avoid SWIG issues
        measurement_vec[0].point_cloud.copyTo(self._points_np)
        
        # Extract valid points: raw_distance > 0 (column 3)
        # Sigma is in column 4
        valid_mask = self._points_np[:, 3] > 0.0
        self._sigma_values = self._points_np[valid_mask, 4].copy()
        self._got_measurement = True

    def onOutputLog(self, verbosity, msg):
        if verbosity > sensorring.LogVerbosity_Debug:
            print("[" + sensorring.LogVerbosityToString(verbosity) + "] " + msg)

    def onStateChange(self, state):
        self._state = state

    def wait_for_new_measurement(self):
        self._got_measurement = False
        while not self._got_measurement and self._state != sensorring.ManagerState_Shutdown:
            time.sleep(0.001)
        return self._sigma_values.copy()


def main():
    print("======================================")
    print("Sigma histogram sensorring example")
    print("======================================")

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

        fig, ax = plt.subplots()
        ax.set_xlabel("Sigma (m)")
        ax.set_ylabel("Count")
        ax.set_title("Sigma of valid ToF points (live)")
        ax.set_ylim(0, 20)
        plt.ion()
        plt.show(block=False)

        # Use fixed bin range so the plot is stable; sigma is in meters, typically small
        sigma_max = 0.003  # 20 mm
        bins = np.linspace(0, sigma_max, 121)  # 60 bins (halved bin size)

        while manager.isMeasuring():
            sigmas = proxy.wait_for_new_measurement()

            if len(sigmas) > 0:
                # Clip to display range for histogram
                sigmas_clipped = np.clip(sigmas, 0, sigma_max)
                ax.clear()
                ax.hist(sigmas_clipped, bins=bins, edgecolor="black", alpha=0.7)
                ax.set_xlabel("Sigma (m)")
                ax.set_ylabel("Count")
                ax.set_title("Sigma of valid ToF points (live)")
                ax.set_ylim(0, 64)
                mean_sigma = np.mean(sigmas)
                std_sigma = np.std(sigmas)
                ax.axvline(mean_sigma, color="red", linestyle="--", label=f"mean = {mean_sigma:.5f} m")
                ax.legend()
                #print(f"Valid points: {len(sigmas)}, sigma mean = {mean_sigma:.5f} m, std = {std_sigma:.5f} m")
            else:
                ax.clear()
                ax.set_xlabel("Sigma (m)")
                ax.set_ylabel("Count")
                ax.set_title("Sigma of valid ToF points (live) – no valid points")
                ax.set_ylim(0, 20)
                print("No valid points in last frame")

            fig.canvas.draw()
            fig.canvas.flush_events()

        manager.stopMeasuring()

    except Exception as e:
        print("Caught:", e)
    finally:
        plt.ioff()
        plt.show(block=True)


if __name__ == "__main__":
    main()
