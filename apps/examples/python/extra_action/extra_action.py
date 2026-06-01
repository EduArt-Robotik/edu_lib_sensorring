#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   extra_action.py
 @author EduArt Robotik GmbH
 @brief  This example demonstrates how to control WS2812b LEDs
         with a smooth color cycling animation.
 @date 2025-11-18
"""

import math
import time
import eduart.sensorring as sensorring


# Default SocketCAN interface (Linux only, expects a SocketCAN interface named "can0")
CAN_INTERFACE_NAME = "can0"
CAN_INTERFACE_TYPE = sensorring.InterfaceType_SocketCan

# Default USBtingo interface (cross-platform, uses the first available USBtingo device)
USBTINGO_INTERFACE_NAME = "0"
USBTINGO_INTERFACE_TYPE = sensorring.InterfaceType_UsbTingo

# Parameters for smooth color cycling of the WS2812b lights.
BRIGHTNESS = 0.2
STEP = 0.15        # Speed of the color change
OFFSET_G = 2.0943951  # 120 degrees phase shift
OFFSET_B = 4.1887902  # 240 degrees phase shift


def to_channel(value):
  """Map a sine wave value to a [0, 255] color channel."""
  v = 0.5 * (math.sin(value) + 1.0)
  return max(0, min(255, int(v * 255.0 + 0.5)))


def main():
  print("===============================")
  print("Extra action sensorring example")
  print("===============================")
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
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.VL53L8CX_Params(), sensorring.WS2812b_Params())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.VL53L8CX_Params(), sensorring.WS2812b_Params())

    # Create the MeasurementManager directly from the factory
    manager = sensorring.MeasurementManager(params, factory)

    # Subscribe to depth sensors for rate tracking
    got_first = [False]
    counter = [0]
    def on_depth_measurement(meas):
      if meas.sensor_index == 0:
        got_first[0] = True
        counter[0] += 1

    depth_sub = manager.depthSensors().subscribe(on_depth_measurement)

    # Get a handle to the lights
    lights = manager.lights()

    # Start the measurements
    manager.startMeasuring()

    while not got_first[0] and manager.isMeasuring():
      time.sleep(0.1)

    if manager.isMeasuring():
      print("\nSensorring successfully initialized.")
      print("\nStart printing animation frames:")

      phase = 0.0
      last_print = time.time()
      while manager.isMeasuring():
        # Advance phase and compute smooth RGB values from three sine waves.
        phase += STEP
        red = int(to_channel(phase) * BRIGHTNESS)
        green = int(to_channel(phase + OFFSET_G) * BRIGHTNESS)
        blue = int(to_channel(phase + OFFSET_B) * BRIGHTNESS)

        # Update the light color via the Light interface (applied in next state-machine cycle)
        for i in range(len(lights)):
          lights[i].setLight(sensorring.LightMode_FixedColor, red, green, blue)

        now = time.time()
        if now - last_print > 1.0:
          print(f"Current frame: {counter[0]}\r", end="", flush=True)
          last_print = now
        time.sleep(0.05)

      depth_sub.cancel()

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
