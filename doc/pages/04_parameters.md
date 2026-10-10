# Configuration Parameters

The Sensor Ring library groups configuration into manager, communication
interface, board, and device parameter structures. This page documents the
public C++ library parameters; parameters specific to ROS wrappers are documented
with the wrapper.

## Applying Parameters

Pass interface parameters to `SensorRingFactory::addInterface()`, board
parameters to `expectBoard()`, and device parameters to `expectDevice()` or
`setDefaultDeviceParams()`. Explicit device parameters supplied with
`expectDevice()` take precedence over defaults. Defaults are useful when a
device is auto-discovered or when an expected device does not specify its own
parameters.

`ManagerParams` is passed to the `MeasurementManager` constructor. Set
parameters before constructing the corresponding object.

For example, these LED defaults apply to discovered WS2812b devices and
expected light devices without explicit parameters:

```cpp
device::LightParams light_params;
light_params.distance_map_lower_m = 0.25;
light_params.distance_map_upper_m = 1.5;
light_params.distance_map_max_brightness = 50;
factory.setDefaultDeviceParams(light_params);
```

## Measurement Manager

`manager::ManagerParams` configures measurement operations:

| Field | Default | Description |
|:------|:--------|:------------|
| `timeout` | `1000 ms` | Timeout for individual sensor operations before the error handler is called. |
| `repair_errors` | `true` | Enables automatic recovery from communication and timing errors. When `false`, the manager shuts down when an error is detected. |

## Communication Interfaces

Pass either `com::SocketCanParams` or `com::UsbTingoParams` to
`SensorRingFactory::addInterface()`. Both inherit the shared CAN settings from
`com::CanParams`; the interface name is inherited from `com::InterfaceParams`.

| Field | Default | Description |
|:------|:--------|:------------|
| `name` | Empty | Interface identifier: for example, `can0` for SocketCAN or a USBtingo device index/serial number. |
| `respond_with_brs` | `false` | Enables CAN FD bit-rate switching for the data phase of messages sent by sensor boards to the host. |
| `data_bitrate` | `0` | CAN FD data-phase bitrate in bits per second, from `1,000,000` to `8,000,000`. Used when either BRS setting is enabled; `0` uses the arbitration bitrate (1 Mbps). |
| `data_sample_point` | `0.0` | Optional data-phase sample point in `[0, 1]`; `0` uses the CAN controller default. |
| `send_with_brs(bool)` | Disabled | Enables or disables BRS for messages sent by the host. The host CAN controller must support transmitter delay compensation (TDC) for reliable operation. |

`send_with_brs()` with no argument returns whether host-to-board BRS is enabled.
SocketCAN is available on Linux; USBtingo settings apply to the USB-to-CAN
adapter.

## Sensor Boards

Pass `board::SensorBoardParams` to `SensorRingFactory::expectBoard()`:

| Field | Default | Description |
|:------|:--------|:------------|
| `board_type` | `Undefined` | Expected hardware board type. `Undefined` keeps all supported device types available for backward compatibility. |
| `orientation` | `Orientation::None` | Mounting orientation used by orientation-sensitive processing, such as light animations and thermal images. |
| `rotation` | `{0, 0, 0}` | Sensor pose rotation as Euler angles in degrees, applied in Roll (X), Pitch (Y), Yaw (Z) order. |
| `translation` | `{0, 0, 0}` | Sensor pose translation as X, Y, Z coordinates in meters. |

`Orientation::Left` mirrors light animations horizontally; `Orientation::Right`
uses them as-is. `Orientation::None` leaves orientation-specific behavior at its
default.

## Device Parameters

All device parameter structures inherit `device::DeviceParams`:

| Field | Default | Description |
|:------|:--------|:------------|
| `id` | Undefined type, index `0` | Device identifier containing its type and instance index. |
| `enable` | `true` | Whether the device is enabled when created. |
| `max_rate_hz` | Device-specific | Maximum measurement rate the device can sustain (Hz, with tenths precision), used by the scheduler to calculate measurement-group divisors. |

Category-level structures add the following fields:

| Structure | Field | Default | Description |
|:----------|:------|:--------|:------------|
| `DepthSensorParams` | — | — | No additional fields beyond `DeviceParams`. |
| `ThermalSensorParams` | `t_min_deg_c` | `20` | Minimum temperature in Celsius for thermal image color mapping when `auto_min_max` is `false`. |
| `ThermalSensorParams` | `t_max_deg_c` | `30` | Maximum temperature in Celsius for thermal image color mapping when `auto_min_max` is `false`. |
| `ThermalSensorParams` | `auto_min_max` | `true` | Automatically scales each image using its coldest and hottest temperatures. |
| `LightParams` | `distance_map_lower_m` | `0.1` | Lower distance-map saturation threshold in meters; distances at or below it map to red. |
| `LightParams` | `distance_map_upper_m` | `0.5` | Upper distance-map saturation threshold in meters; distances at or above it map to green, with interpolation between the thresholds. |
| `LightParams` | `distance_map_max_brightness` | `50` | Peak LED channel value in `[0, 255]`; `0` disables the distance indicator. |

### Concrete Device Parameters

Concrete parameter types inherit their category-level fields and use these
maximum measurement rates by default:

| Structure | Default `max_rate_hz` | Additional fields |
|:----------|:---------------------|:------------------|
| `VL53L8CX_Params` | `15` | None beyond `DepthSensorParams`. |
| `TMF8829_Params` | `30` | `resolution_mode`, `k_iterations`, and `result_format` (described below). |
| `HTPA32_Params` | `5` | EEPROM and calibration file settings (described below). |
| `WS2812b_Params` | `15` | None beyond `LightParams`. |

#### TMF8829

| Field | Default | Description |
|:------|:--------|:------------|
| `resolution_mode` | `Res8x8` | Sensor resolution mode. Available modes include 8×8, 16×16, 32×32, and 48×32, with long-range or high-accuracy variants where supported. |
| `k_iterations` | `0` | Iterations per measurement in kilo-iterations; `0` selects the sensor default. |
| `result_format.full_noise` | `false` | Reports unscaled noise strength rather than dividing it by the number of bins. |
| `result_format.xtalk` | `false` | Includes the crosstalk value. |
| `result_format.noise_strength` | `false` | Includes the noise-strength value. |
| `result_format.signal_strength` | `false` | Includes the signal-strength value. |
| `result_format.nr_of_peaks` | `1` | Maximum reported peaks per depth pixel. Valid values are `0`–`4`. |

The selected resolution and result format determine the result frame size. The
library limits each frame to 8192 bytes; use `isResultSizeValid()` to check a
parameter combination.

#### HTPA32

| Field | Default | Description |
|:------|:--------|:------------|
| `use_eeprom_file` | `false` | Reuse cached EEPROM contents from a local file. |
| `use_calibration_file` | `false` | Reuse cached calibration data from a local file. |
| `eeprom_dir` | Empty | Directory used for the EEPROM cache file. |
| `calibration_dir` | Empty | Directory used for the calibration cache file. |
| `eeprom_timeout` | `5000 ms` | Timeout for EEPROM reads during device configuration. |

When enabling either cache option, the process must have read and write access
to the configured directory.

#### WS2812b Distance Map

`LightParams` settings apply to both `device::LightParams` and
`device::WS2812b_Params`. The range must be finite, satisfy
`0 <= lower < upper`, and remain distinct when represented as float32.
Invalid ranges and brightness values outside `[0, 255]` throw
`std::invalid_argument` during parameter validation.

During configuration, the library sends and verifies the thresholds and
brightness, then restores the light mode. It reapplies them after board resets.
Updated firmware is required for read-back verification; legacy firmware
without the corresponding getters fails configuration.

`WS2812b_Device` also provides runtime methods:
`setDistanceMapRange()` / `getDistanceMapRange()` and
`setDistanceMapMaxBrightness()` / `getDistanceMapMaxBrightness()`. They return
`bool`. Setters log invalid input and return `false` without transmitting it.
Range read-back accepts differences of at most 1 micrometer absolute or 1 ppm
relative (whichever is larger); brightness must match exactly. Transmission
failures, malformed replies, mismatches, or missing replies (100 ms timeout)
are logged and return `false`.

Range verification reads the two float32 meter values (lower, upper) using
`PARAMETER_GET_DISTANCE_MAP_RANGE` (`0x90`); brightness verification reads one
byte using `PARAMETER_GET_DISTANCE_MAP_MAX_BRIGHTNESS` (`0x91`). Both get
requests have an empty payload. The brightness value is sent as one unsigned
byte with `PARAMETER_SET_DISTANCE_MAP_MAX_BRIGHTNESS` (`0x11`).

Successful setters update the desired parameters returned by `getParams()`,
preserving the requested range for later resets. Getters query live board values
without changing desired parameters and leave output arguments unchanged on
failure.

<div class="section_buttons">

| Read Previous | Read Next |
|:--|--:|
| [Software](03_software.md) | [Examples](05_examples.md) |

</div>
