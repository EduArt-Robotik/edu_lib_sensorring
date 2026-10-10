# Software

## 1. Public Interface of the Sensor Ring Library

### 1.1 Measurement Interface

The public interface has **two measurement related components**:

- The **MeasurementManager**:<br>
  This class executes the measurements, collects them and distributes them to all registered subscribers. It is responsible for the timing of the measurement process. The measurements can either be run asynchronously in a separate thread with the `startMeasuring()` and `stopMeasuring()` methods, or in the users thread by repeatedly calling the `measureSome()` method.<br>
  Clients subscribe to measurements via typed device interfaces (`depthSensors()`, `thermalSensors()`, `lights()`) and to state changes via `subscribeToStateChanges()`. Each call returns a `Subscription` object — the callback stays active for as long as the `Subscription` is alive.

- The **ManagerParams**<br>
  This parameter set controls the runtime behavior of the `MeasurementManager`.
  See [Configuration Parameters](04_parameters.md) for its fields and defaults.

### 1.2 Logger Interface

In addition to the measurement related interface the library provides a **logger interface**:

- The **Logger**<br>
  The Logger is a singleton that collects all debug, info and error messages that are raised internally. Clients subscribe to log output via `Logger::getInstance()->subscribe()`, which returns a `Subscription` object.<br>
  It is recommended to subscribe to the Logger **before** creating other library objects so that messages emitted during initialization are not lost (see the [Examples](05_examples.md#3-logger-subscription) section for details).

## 2. Topology of the System

The EduArt Sensor Ring collects measurements from sensor boards connected
through one or more communication interfaces. The library represents this
topology with the following components:

- A `SensorRingFactory` discovers boards on configured interfaces and builds
  the ring.
- A `SensorRing` contains one `SensorBus` for each communication interface.
- Each `SensorBus` contains the `SensorBoard` instances on that interface;
  each board owns its configured devices.
- A `MeasurementManager` operates on the ring and provides typed device groups
  and state-change subscriptions to applications.

## 3. Sensor Ring Factory

The `SensorRingFactory` is the primary way to create a `SensorRing` instance.
It handles hardware discovery, board validation and device instantiation in a single `build()` call.

### 3.1 Basic Usage

The minimal workflow is:

1. Create a factory instance
2. Register communication interfaces with `addInterface()`
3. Optionally declare expected boards with `expectBoard()`
4. Call `build()` to enumerate hardware and construct the `SensorRing`

```cpp
using namespace eduart::sensorring;

com::UsbTingoParams interface{ "0" };
SensorRingFactory factory(ValidationMode::Relaxed);
factory.addInterface(interface);
auto sensor_ring = factory.build();
```

The returned `SensorRing` is passed to the `MeasurementManager` as before:

```cpp
manager::ManagerParams params;
auto manager = std::make_unique<manager::MeasurementManager>(params, std::move(sensor_ring));
```

### 3.2 Validation Modes

The validation mode is selected when constructing `SensorRingFactory` (it
defaults to `ValidationMode::Relaxed`). `build()` uses that mode to match
expectations against discovered hardware.

| Mode | Behaviour |
|:-----|:----------|
| **Strict** | Expectations are matched 1:1 by index against discovered boards. Any mismatch in board type, device type or board count returns `nullptr`. |
| **Relaxed** | For each expectation the factory **searches** all unclaimed boards for the first compatible one. Boards that are not claimed by any expectation remain unconfigured. Mismatches are logged as warnings. |

In strict mode the order and count of `expectBoard()` calls must match the physical bus exactly.
In relaxed mode the factory finds compatible boards regardless of their position on the bus.

### 3.3 Configuring Board Expectations

**Auto-discovery** (no expectations): every board found on the bus is used with default configuration.

```cpp
auto sensor_ring = factory.build();
```

**Board type constraint**: declare the expected board type, then declare the
devices to instantiate on it.

```cpp
using namespace eduart::sensorring;

board::SensorBoardParams board_params;
board_params.board_type = board::SensorBoardType::Headlight;
factory.expectBoard(board_params);
factory.expectDevice(device::HTPA32_Params{});
```

**Explicit device params**: pass device parameters after `expectBoard()`. Only
the declared device types are instantiated on that board.

```cpp
using namespace eduart::sensorring;

factory.expectBoard({});
factory.expectDevice(device::HTPA32_Params{});
```

**Default device params**: applied to every device of that type when no explicit params are given.

```cpp
using namespace eduart::sensorring;

device::VL53L8CX_Params tof_params;
tof_params.max_rate_hz = 10.0;
factory.setDefaultDeviceParams(tof_params);
```

### 3.4 Enumeration and Topology

The factory can enumerate hardware without building a `SensorRing`:

```cpp
auto results = factory.enumerate();
std::cout << factory.printTopology();
```

The `EnumerationMap` returned by `enumerate()` or `getLatestEnumerationResult()` maps each `ComInterfaceID` to a vector of `EnumerationInformation` structs that report board type, connection state, configuration state and available device types.

## 4. Configuration Parameters

The library exposes parameter structures for the measurement manager,
communication interfaces, boards, and devices. See the
[Configuration Parameters](04_parameters.md) page for their fields, defaults,
and how to apply them.

<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Installation](02_installation.md) | [Configuration Parameters](04_parameters.md) |

</div>