# Input parameters

## MeasurementManager

**manager::ManagerParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|bool               | print_topology        | If set to `true` a formatted string describing the sensor<br>topology is output via the `onLogOutput()` callback once<br> when the MeasurementManager `init()` method is called.| `true`, `false` |
|double             | frequency_tof_hz      | Sets the desired frequency for Time-of-Flight sensor<br>measurements. The maximum measurement frequency<br> of the VL53L8CX Sensor is 15 hz in 8 × 8 mode. The maximum<br> frequency of the Sensorring is a little lower than that. If set to `0.0`<br> the system tries to fetch measurements as fast as possible.| `0.0 ... 15.0`|
|double             | frequency_thermal_hz  | Sets the desired frequency for thermal sensor measurements.<br> The maximum measurement frequency of the HTPA32 Sensor is 5 Hz.<br> The real frequency is limited to discrete values, because the Time-of-<br>Flight sensor measurements are prioritized over the thermal measurements. | `0.0 ... 5.0` |
|ring::RingParams   | ring_params           | Parameter set of the SensorRing object, that is managed by the<br> MeasurementManager.| n.a. |


## SensorRing

**ring::RingParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|std::chrono::milliseconds | timeout | Timeout used internally to wait for responses and measurements<br> from the sensors. Values below 200 ms may be too short and could<br> cause errors. | `200 ... 1000` ms |
|std::vector\<bus::BusParams\> | bus_param_vec | One sensor ring may use an arbitrary number of communication<br> interfaces to connect sensor boards. Each communication interface<br> has its own parameter set. | n.a. |

## SensorBus

**bus::BusParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|std::string | interface_name | Name of the communication interface to be opened.<br> Currently only can interfaces are supported. | n.a. |
|std::vector\<sensor::SensorBoardParams\> | board_param_vec | One sensor bus may use an arbitrary number of sensor boards<br> connected. Each sensor board has its own parameter set. | n.a. |

## SensorBoard

**sensor::SensorBoardParams**
| Type | Parameter name | Valid values |
|---|---|---|
| std::size_t | idx |   |
| bool | enable_tof |   |
| bool | enable_thermal |   |
| sensor::TofSensorParams | thermal_params |   |
| sensor::ThermalSensorParams | tof_params |   |
| sensor::LightParams | led_params |   |

### TofSensor

**sensor::TofSensorParams**
| Type | Parameter name | Valid values |
|---|---|---|
| math::Vector3 | rotation |   |
| math::Vector3 | translation |   |

### ThermalSensor

**sensor::ThermalSensorParams**
| Type | Parameter name | Valid values |
|---|---|---|
| double | t_min_deg_c |   |
| double | t_max_deg_c |   |
| bool | auto_min_max |   |
| bool | use_eeprom_file |   |
| bool | use_calibration_file |   |
| std::string | eeprom_dir |   |
| std::string | calibration_dir |   |
| math::Vector3 | rotation |   |
| math::Vector3 | translation |   |
| sensor::SensorOrientation | orientation |   |

### LedLights

**sensor::LightParams**
| Type              | Parameter name        | Description | Valid values |
|---|---|---|---|
|std::size_t | nr_of_leds | Number of leds of a circuit board. | n.a. |
|sensor::SensorOrientation | board_param_vec | Specifies the orientation of a sensor board. Used to rotate<br> the thermal images and to run light animations in the<br> correct orientation. | `left`, `right`, `none` |