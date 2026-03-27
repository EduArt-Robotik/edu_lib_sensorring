#!/usr/bin/python3

# Copyright (c) 2026 EduArt Robotik GmbH

"""
 @file   extra_action.py
 @author EduArt Robotik GmbH
 @brief  This example demonstrates how to use enqueueExtraAction() to control WS2812b LEDs
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
    # Create SensorRing via factory auto-discovery
    factory = sensorring.SensorRingFactory()
    factory.addInterface(can_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.VL53L8CX_Params(), sensorring.WS2812b_Params())
    factory.addInterface(usbtingo_interface)
    factory.expectBoard(sensorring.SensorBoardParams(), sensorring.VL53L8CX_Params(), sensorring.WS2812b_Params())
    sensor_ring = factory.build(sensorring.ValidationMode_Relaxed)

    if sensor_ring is None:
      print("Failed to create SensorRing. Exiting.")
      return

    # Create the MeasurementManager with the SensorRing
    manager = sensorring.MeasurementManager(params, sensor_ring)

    # Subscribe to ToF device group for rate tracking
    got_first = [False]
    counter = [0]
    tof_sub = manager.subscribeToDeviceGroup(
      sensorring.DeviceType_VL53L8CX,
      lambda group: (got_first.__setitem__(0, True), counter.__setitem__(0, counter[0] + 1))
    )

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

        # Update the light color via the extra action interface so it runs in the MeasurementManager context.
        manager.enqueueExtraAction(
          lambda r=red, g=green, b=blue: sensorring.WS2812b_setLight(sensorring.LightMode_FixedColor, r, g, b)
        )

        now = time.time()
        if now - last_print > 1.0:
          print(f"Current frame: {counter[0]}\r", end="", flush=True)
          last_print = now
        time.sleep(0.05)

      tof_sub.cancel()

      # Stop the measurements
      manager.stopMeasuring()

  except Exception as e:
    print(f"Caught: {e}")


if __name__ == "__main__":
  main()
