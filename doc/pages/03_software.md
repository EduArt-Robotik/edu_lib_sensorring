# Software

## 2. Sensor topology

The EduArt Sensorring is a sensor system that collects and combines measurements from multiple individual sensors. The individual sensors are daisy chained together in series and share a communication interface and a power supply. The chain of sensor is terminated by a master device at one end e.g. a Raspberry Pi or a CAN to USB converter. It is possible to use multiple communication interfaces to distribute the sensor data and enable the use of more sensors simultaneously. The following class diagram illustrates the topology and the reflection of the hardware layers in the library.

<img src="doc/images/class_diagram_simple.png" width="900" onerror="this.onerror=null; this.src='class_diagram_simple.png';">

## 3. Input Parameters

Each of the core classes of the library has its own parameter set which is shown in the above class diagram. The parameters have to be initialized externally and passed to the MeasurementManager upon initialization. The individual parameters are described [here](doc/pages/parameters.md). See section _3. Software interface of the library_ for details on how the parameter structures are implemented.

## 4. Software interface of the library

The main software interface of the Sensorring library is defined in three header files which are installed with the library.
- [MeasurementManger.hpp](include/sensorring/MeasurementManager.hpp)
- [MeasurementClient.hpp](include/sensorring/MeasurementClient.hpp)
- [Parameters.hpp](include/sensorring/Parameters.hpp)




## 6. Supported languages

### C++
The library is written in c++ and primarily designed to be used as such.

### Python
The library can be built with `-DBUILD_PYTHON_BINDINGS=ON` option to generate python bindings.


## 7. Using the library in a custom project
The library includes both C++ and Python [examples](https://github.com/EduArt-Robotik/edu_lib_sensorring/tree/master/apps/examples) that show how to create a minimal program using the Sensor Ring.
For further references on how to use the library in custom projects refer to the [Ros](https://github.com/EduArt-Robotik/edu_sensorring_ros1) and [Ros2](https://github.com/EduArt-Robotik/edu_sensorring_ros2) wrappers of this library.
Both show how to include the library in larger CMake projects.





# Input parameters

## MeasurementManager

**manager::ManagerParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|bool               | print_topology        | If set to `true` a formatted string describing the sensor topology is output via the `onLogOutput()` callback once when the MeasurementManager `init()` method is called.| `true`, `false` |
|double             | frequency_tof_hz      | Sets the desired frequency for Time-of-Flight sensor measurements. The maximum measurement frequency of the VL53L8CX Sensor is 15 hz in 8 × 8 mode. The maximum frequency of the whole system is a little lower than that. If set to `0.0` the system tries to fetch measurements as fast as possible.| `0.0 ... 15.0`|
|double             | frequency_thermal_hz  | Sets the desired frequency for thermal sensor measurements. The maximum measurement frequency of the HTPA32 Sensor is 5 Hz. The real frequency is limited to discrete values, because the Time-of- Flight sensor measurements are prioritized over the thermal measurements. | `0.0 ... 5.0` |
|ring::RingParams   | ring_params           | Parameter set of the SensorRing object, that is managed by the MeasurementManager.| n.a. |


## SensorRing

**ring::RingParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|std::chrono::milliseconds | timeout | Timeout used internally to wait for responses and measurements from the sensors. Values below 200 ms may be too short and could cause errors. | `200 ... 1000` ms |
|std::vector\<bus::BusParams\> | bus_param_vec | One sensor ring may use an arbitrary number of communication interfaces to connect sensor boards. Each communication interface has its own parameter set. | n.a. |

## SensorBus

**bus::BusParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|std::string | interface_name | Name of the communication interface to be opened. Currently only can interfaces are supported. | n.a. |
|std::vector\<sensor::SensorBoardParams\> | board_param_vec | One sensor bus may use an arbitrary number of sensor boards connected. Each sensor board has its own parameter set. | n.a. |

## SensorBoard

**sensor::SensorBoardParams**
| Type | Parameter name  | Description | Valid values |
|---|---|---|---|
| sensor::TofSensorParams | thermal_params | Parameters of the thermal sensor on the sensor board. Leave empty if no thermal sensor is placed on the board. | n.a. |
| sensor::ThermalSensorParams | tof_params | Parameters of the Time-of-Flight sensor on the sensor board. Leave empty if no Time-of-Flight sensor is placed on the board. | n.a. |
| sensor::LightParams | led_params | Parameters of the lights on the sensor board. Leave empty if no leds are placed on the board. | n.a. |

### TofSensor

**sensor::TofSensorParams**
| Type | Parameter name | Description | Valid values |
|---|---|---|---|
| bool | enable | Enable flag for the Time-of-Flight sensor. Used to disable the sensor when no Time-of-Flight measurements should be performed. | `true`, `false` |
| math::Vector3 | rotation | 3 dimensional vector with euler angles specifying the sensor orientation. The elements are [`rx`, `ry`, `rz`] and describe the rotation around the cartesian coordinate axes `x`, `y` and `z` in `degrees`. | [`0..2π`, `0..2π`, `0..2π`]|
| math::Vector3 | translation | 3 dimensional vector with offset distances specifying the sensor position in relation to a common reference point. The elements are [`x`, `y`, `z`] and describe the translation along the cartesian coordinate axes `x`, `y` and `z` in `meters`. | [`...`, `...`, `...`] |

### ThermalSensor

**sensor::ThermalSensorParams**
| Type | Parameter name | Description  | Valid values |
|---|---|---|---|
| bool | enable | Enable flag for the thermal sensor. Used to disable the sensor when no thermal measurements should be performed. | `true`, `false` |
| double | t_min_deg_c | Minimum temperature for scaling the grayscale and falsecolor images. Only used when the `auto_min_max` parameter is set to `false`. | `...` |
| double | t_max_deg_c | Maximum temperature for scaling the grayscale and falsecolor images. Only used when the `auto_min_max` parameter is set to `false`. | `...` |
| bool | auto_min_max | Flag to switch between automatic and manual scaling of the grayscale and falsecolor images. When set to `true` the images are scaled dynamically between the minimum and maximum temperature value in the image. When set to `false` the images are scaled dynamically between the values of the `t_min_deg_c` and `t_min_deg_c` parameters. When using manual scaling all out-of-bounds values are clipped. | `true`, `false` |
| bool | use_eeprom_file | Flag to enable saving the conversion values of the thermal sensors in a file. They are stored on the EEPROM of the sensors and read at start-up. When the parameter is  set to `true` the values are read once and stored in a local file. When set to `false`or a local file does no longer exists the calibration values are read from the sensor. | `true`, `false` |
| bool | use_calibration_file | Flag to enable saving calibration images for the thermal sensors. | `true`, `false` |
| std::string | eeprom_dir | Directory where the files with the conversion values from the thermal sensors are stored. It is recommended to use absolute paths. | valid paths |
| std::string | calibration_dir | Directory where the files with the calibration images from the thermal sensors are stored. It is recommended to use absolute paths.| valid paths |
| math::Vector3 | rotation | 3 dimensional vector with euler angles specifying the sensor orientation. The elements are [`rx`, `ry`, `rz`] and describe the rotation around the cartesian coordinate axes `x`, `y` and `z` in `degrees`. | [`0..2π`, `0..2π`, `0..2π`]|
| math::Vector3 | translation | 3 dimensional vector with offset distances specifying the sensor position in relation to a common reference point. The elements are [`x`, `y`, `z`] and describe the translation along the cartesian coordinate axes `x`, `y` and `z` in `meters`. | [`...`, `...`, `...`] |
|sensor::SensorOrientation | orientation | Specifies the orientation of a sensor board. Used to rotate the thermal images to the correct orientation. | `left`, `right`, `none` |

### LedLight

The light functionality is not fully implemented yet and there is currently no interface to access them. They are synchronized once when the MeasurementManager state machine starts and run a predefined light animation. 

**sensor::LightParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
| bool | enable | Enable flag for the lights. Used to disable the lights. *(Currently not implemented)* | `true`, `false` |
|sensor::SensorOrientation | orientation | Specifies the orientation of a sensor board. Used to run light animations in the right direction. | `left`, `right`, `none` |


<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Installation](02_installation.md) | [Developer Documentation](04_dev_doc.md) |

</div>