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
| sensor::LightParams | led_params | Parameters of the led lights on the sensor board. Leave empty if no leds are placed on the board. | n.a. |

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
| bool | enable | Enable flag for the led lights. Used to disable the lights. *(Currently not implemented)* | `true`, `false` |
|sensor::SensorOrientation | orientation | Specifies the orientation of a sensor board. Used to run light animations in the right direction. | `left`, `right`, `none` |