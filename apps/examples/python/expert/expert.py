#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   expert.py
 @author EduArt Robotik GmbH
 @brief  Expert example demonstrating expert-user features of the SensorRing library.

         This example shows how to:
         - Enumerate hardware before building
         - Configure per-board poses and per-device params
         - Use per-device subscriptions alongside group subscriptions
         - Form custom spatial subgroups from selected devices
         - Monitor state changes for diagnostics
         - Control lights via the action queue

 @date 2026-04-21
"""

import time
import threading
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SocketCan

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_UsbTingo


def main():
  print("================================")
  print("Expert sensorring example")
  print("================================")
  print()

  # --- Logger ---
  log_sub = sensorring.Logger.getInstance().subscribe(
    lambda verbosity, msg:
      print(f"[{sensorring.LogVerbosityToString(verbosity)}] {msg}")
      if verbosity > sensorring.LogVerbosity_Debug else None
  )

  try:
    # =========================================================================
    # 1. Configure interfaces
    # =========================================================================
    can_interface = sensorring.ComInterfaceID()
    can_interface.type = CAN_INTERFACE_TYPE
    can_interface.name = CAN_INTERFACE_NAME

    usbtingo_interface = sensorring.ComInterfaceID()
    usbtingo_interface.type = USBTINGO_INTERFACE_TYPE
    usbtingo_interface.name = USBTINGO_INTERFACE_NAME

    # =========================================================================
    # 2. Configure the factory with explicit board poses and device params
    # =========================================================================

    # Use Relaxed validation so the build succeeds even if not all
    # expected boards are physically present.
    factory = sensorring.SensorRingFactory(sensorring.ValidationMode_Relaxed)

    factory.addInterface(can_interface)

    # Board 0: Front-left, rotated 45 deg around Z.
    vl53_params = sensorring.VL53L8CX_Params()
    htpa_params = sensorring.HTPA32_Params()
    htpa_params.auto_min_max = True
    ws_params = sensorring.WS2812b_Params()

    board_0 = sensorring.SensorBoardParams()
    board_0.rotation = sensorring.Vector3(0, 0, 45)
    board_0.translation = sensorring.Vector3(0.1, 0.05, 0)
    factory.expectBoard(board_0, vl53_params, htpa_params, ws_params)

    # Board 1: Front-right, rotated -45 deg around Z.
    board_1 = sensorring.SensorBoardParams()
    board_1.rotation = sensorring.Vector3(0, 0, -45)
    board_1.translation = sensorring.Vector3(0.1, -0.05, 0)
    factory.expectBoard(board_1, vl53_params, htpa_params, ws_params)

    # Second interface (if available).
    factory.addInterface(usbtingo_interface)
    board_2 = sensorring.SensorBoardParams()
    board_2.rotation = sensorring.Vector3(0, 0, 0)
    board_2.translation = sensorring.Vector3(-0.1, 0, 0)
    factory.expectBoard(board_2, vl53_params, htpa_params, ws_params)

    # =========================================================================
    # 3. Enumerate hardware (optional - useful for diagnostics)
    # =========================================================================
    factory.enumerate()
    print("\n--- Hardware Topology ---")
    print(factory.printTopology())

    # =========================================================================
    # 4. Build via the factory constructor (standard path)
    #    The unique_ptr<SensorRing> constructor is C++ only.
    # =========================================================================
    params = sensorring.ManagerParams()
    params.frequency_tof_hz = 10.0      # Cap ToF rate to 10 Hz
    params.frequency_thermal_hz = 4.0   # Cap thermal rate to 4 Hz
    params.repair_errors = True

    manager = sensorring.MeasurementManager(params, factory)

    # =========================================================================
    # 5. Retrieve typed device groups
    # =========================================================================
    all_depth = manager.depthSensors()
    all_thermal = manager.thermalSensors()
    all_lights = manager.lights()

    print(f"\nDiscovered devices:")
    print(f"  Depth sensors:   {all_depth.size()}")
    print(f"  Thermal sensors: {all_thermal.size()}")
    print(f"  Lights:          {all_lights.size()}")
    print()

    # =========================================================================
    # 6. Per-device subscriptions (each sensor gets its own callback)
    # =========================================================================
    per_device_subs = []

    def make_depth_callback(sensor_idx):
      """Create a per-sensor callback. Useful for sector-specific processing."""
      def cb(meas):
        # Per-sensor processing, e.g. obstacle detection for a specific sector.
        # meas.transformed_point_cloud contains points in world frame.
        pass
      return cb

    for i in range(all_depth.size()):
      per_device_subs.append(all_depth[i].subscribe(make_depth_callback(i)))

    # =========================================================================
    # 7. Custom spatial subgroup (if we have at least 2 depth sensors)
    # =========================================================================
    front_sub = None
    if all_depth.size() >= 2:
      front_depth = sensorring.DepthSensorGroup([all_depth[i] for i in range(2)])

      def front_callback(meas):
        # Fires for each sensor in the subgroup.
        # Combine them for a wider front FOV, run collision checks, etc.
        pass

      front_sub = front_depth.subscribe(front_callback)
      print(f"Front depth subgroup: {front_depth.size()} sensors")

    # =========================================================================
    # 8. Group subscription for thermal (per-sensor callbacks)
    # =========================================================================
    thermal_frame_count = [0]
    lock = threading.Lock()

    def thermal_callback(meas):
      if meas.sensor_index == 0:
        with lock:
          thermal_frame_count[0] += 1
      # meas.temperatures holds the 32x32 temperature array in deg C.
      # meas.min_deg_c / meas.max_deg_c give the frame extremes.

    thermal_sub = all_thermal.subscribe(thermal_callback)

    # =========================================================================
    # 9. Set initial light state via the action queue
    # =========================================================================
    for i in range(all_lights.size()):
      all_lights[i].setLight(sensorring.LightMode_Pulsation, 0, 0, 0)

    # =========================================================================
    # 10. Start the threaded measurement loop
    # =========================================================================
    manager.startMeasuring()

    start = time.time()
    while manager.isMeasuring() and (time.time() - start < 10.0):
      with lock:
        count = thermal_frame_count[0]
      print(f"Thermal frames: {count}\r", end="", flush=True)
      time.sleep(1.0)

    # Switch lights to fixed green before shutdown.
    for i in range(all_lights.size()):
      all_lights[i].setLight(sensorring.LightMode_FixedColor, 0, 128, 0)
    time.sleep(0.5)  # Give state machine one cycle to drain the queue.

    manager.stopMeasuring()

    # =========================================================================
    # 11. Clean shutdown - subscriptions cancel automatically via RAII,
    #     but explicit cancellation is also supported:
    # =========================================================================
    thermal_sub.cancel()
    if front_sub:
      front_sub.cancel()
    for s in per_device_subs:
      s.cancel()
    log_sub.cancel()

    print(f"\nTotal thermal frames received: {thermal_frame_count[0]}")

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
