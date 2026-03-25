# Examples

The Sensor Ring library includes both [C++ examples](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp) and [Python examples](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/python) that show how to use it in custom projects.

> ⚠️ **Interface:** All examples register both a **SocketCAN** and an **USBtingo** interface by default. The `SensorRingFactory` auto-discovery will use whichever interface is available on your system. Depending on your setup, you may need to adjust the interface names (e.g. the SocketCAN device name or the USBtingo device index) in the respective example source file. Note that SocketCAN is only available on Linux.


## 1. C++ <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp"><img src="https://img.shields.io/badge/C++-00599C?logo=cplusplus&logoColor=white" alt="C++"></a>

The library is written in C++ and it is recommended to use the C++ interface of the library for performance reasons.

The following examples show how to use the Sensor Ring library in your own C++ project:

### Measurement Rate Examples

The first three examples are **functionally identical** — they all use the `SensorRingFactory` for auto-discovery and display the current ToF and thermal measurement rate on the command line. They differ only in the programming pattern used to receive measurements:

- [Minimal Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/minimal/src/main.cpp) (**function-based**): Subscribes to device groups and state changes using lambda callbacks directly on the `MeasurementManager`
- [Proxy Class Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/proxy_class/src/main.cpp) (**object-oriented**): Wraps the subscription logic in a custom proxy class that binds its member functions as callbacks via `std::bind`
- [Client Interface Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/client_interface/src/main.cpp) (**client interface**): Inherits from the optional `MeasurementClient` and `LoggerClient` interfaces and overrides their virtual callback methods

### Visualization and Action Examples

- [Depth Map Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/depth_map/src/main.cpp): Prints a colored 8×8 depth map of the first connected ToF sensor on the command line
- [Thermal Map Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/thermal_map/src/main.cpp): Prints a 32×32 false-color thermal image from the first connected HTPA32 sensor on the command line
- [Extra Action Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/extra_action/src/main.cpp): Demonstrates how to use `enqueueExtraAction()` to control WS2812b LEDs with a smooth color cycling animation

> ⚠️ To use the `depth_map` or `thermal_map` C++ examples on Windows you might first need to enable UTF-8 support for your current terminal session with this command:<br/>
`$OutputEncoding = [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding`.

## 2. Python <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/python"><img src="https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=white" alt="Python"></a>
> ℹ️ The library can be built with `-DSENSORRING_BUILD_PYTHON_BINDINGS=ON` option to generate python bindings.

> ⚠️ To use the `sensorring` python package you have to append the location of the package to your `PYTHONPATH` environment variable.

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">
    ```sh
    export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3/dist-packages
    ```
    
  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">
    ```ps
    $env:PYTHONPATH = "${env:PYTHONPATH};C:\Program Files\EduArt Robotik GmbH\Sensor Ring\bindings\python3"
    $env:EDU_SENSORRING_DIR = "C:\Program Files\EduArt Robotik GmbH\Sensor Ring\"
    ```
    Alternatively you can use the Windows `Edit the System Environment Variables` tool to add the python package location to the environment variable `PYTHONPATH`.

  </div>
</div>

The following examples show how to use the Sensor Ring library in your own Python project:

- [Minimal Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/minimal/src/main.cpp): Displays the current measurement rate
- [Depth Map Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/depth_map/src/main.cpp): Displays a depth map of the ToF measurement on the command line
- [Depth View Example](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp/depth_view/src/main.cpp): Displays a 3D plot of the ToF measurement on the command line using [matplotlib](https://matplotlib.org/)

<div align=center>
<table style="border: none;">
<tr>
  <td style="text-align:center">
    <img src="../images/example_cpp1.webp" height=500 onerror="this.onerror=null; this.src='example_cpp1.webp';"><br>
    The `depth_map` example
  </td>
  <td style="text-align:center">
    <img src="../images/example_py1.webp" height=500 onerror="this.onerror=null; this.src='example_py1.webp';"><br>
    The `depth_view` example.
  </td>
</tr>
</table>
</div>

The use of the Python interface is similar to that of the C++ interface with a few exceptions that are explained below.

C++ has the two client interface classes `MeasurementClient` and `LoggerClient`.
With the generated Python bindings it is not possible to inherit from both base classes in one python class simultaneously.
Only the first base class is handled correctly, the second one is not recognized correctly and throws an error when trying to registering it.
For this reason the Python interface has the additional `SensorringClient` class, which combines the callbacks from both `MeasurementClient` and `LoggerClient` in one class.

> ⚠️ Use the `SensorringClient` base class in Python to inherit from both `MeasurementClient` and `LoggerClient`.

> ⚠️ It is strongly recommended to clone measurements to numyp arrays before manipulating them. This is shown in the `onRawTofMeasurement()` callback below.

Below is a minimal example that shows the Python specialities discussed above:

```python
import numpy as np
import eduart.sensorring as sensorring

class MeasurementProxy(sensorring.SensorringClient):
  def __init__(self):
    # Initialize base class
    super().__init__()
    self._points_np = np.zeros((64, 6), dtype=np.float64)

  # Base class callback
  def onRawTofMeasurement(self, measurement_vec):
    measurement_vec[0].point_cloud.copyTo(self._points_np)
  
  # Base class callback
  def onOutputLog(self, verbosity, msg):
    print("[" + sensorring.LogVerbosityToString(verbosity) + "] " + msg)


def main():
  # Create the parameter structure that is used to instantiate the sensorring
  params = sensorring.ManagerParams()
  # (Actually configure the parameters here...)

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
      # (Actually do something useful here ...)
      pass

    # Stop the measurements
    manager.stopMeasuring()

  except Exception as e:
    print("Caught: ", e)

if __name__ == "__main__":
    main()
```

<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Software](03_software.md) | [Wrappers](06_wrappers.md) |

</div>
