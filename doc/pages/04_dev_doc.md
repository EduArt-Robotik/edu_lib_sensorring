# Developer Documentation

## Architecture Overview

The Sensor Ring library follows a layered architecture with clear separation between public API, internal implementation, and hardware abstraction layers. The system is organized hierarchically from high-level measurement management down to individual sensor hardware interfaces.

## Component Hierarchy

The runtime object hierarchy is:

```
SensorRingFactory
    └── SensorRing
        └── SensorBus (one per communication interface)
            └── SensorBoard (one per physical board)
                └── Device

MeasurementManager (public API)
    └── MeasurementManagerImpl (scheduler and state machine)
        └── SensorRing
```

### Component Responsibilities

- **SensorRingFactory**: Configures interfaces and optional board/device expectations, enumerates hardware, and builds a ring.
- **MeasurementManager**: Public measurement-control interface. It can build a ring from a configured factory or take ownership of a pre-built ring.
- **MeasurementManagerImpl**: Internal scheduler and state-machine implementation.
- **SensorRing**: Top-level container managing multiple sensor buses
- **SensorBus**: Manages sensor boards on a single communication interface
- **SensorBoard**: Represents a physical board and owns its configured devices
- **Device**: Base class for concrete devices, including VL53L8CX, TMF8829, HTPA32, and WS2812b

## Directory Structure

```
include/sensorring/          # Public API headers
    ├── SensorRingFactory.hpp
    ├── SensorRing.hpp
    ├── SensorBus.hpp
    ├── board/
    ├── device/
    ├── firmware/
    ├── interface/
    ├── logger/
    ├── manager/
    ├── math/
    ├── measurement/
    └── subscription/

src/                         # Implementation
    ├── SensorRing.cpp
    ├── SensorBus.cpp
    ├── SensorRingFactory.cpp
    ├── board/
    ├── device/
    ├── factory/
    ├── firmware/
    ├── interface/
    ├── logger/
    ├── manager/
    ├── math/
    ├── measurement/
    └── utils/
```

## Design Patterns

### Publisher / Subscription Pattern

The library uses a token-based publish-subscribe pattern for decoupled communication:

- **Publisher<Args...>**: A template that allows any component to publish typed events. Subscribers receive a `Subscription` RAII handle — the callback is automatically unregistered when the handle is destroyed.
- **MeasurementManager**: Publishes per-device measurements via typed accessors (`depthSensors()`, `thermalSensors()`, `lights()`) and state changes via `subscribeToStateChanges()`.
- **Logger**: Publishes log messages via `Logger::getInstance()->subscribe()`.
- **Endpoint-filtered communication**: Internally, sensor boards and devices receive CAN messages through endpoint-filtered subscriptions on the communication interface.

### PIMPL Idiom

The `MeasurementManager` uses the PIMPL (Pointer to Implementation) pattern to hide implementation details:

```cpp
class MeasurementManager {
private:
    std::unique_ptr<MeasurementManagerImpl> _mm_impl;
};
```

This allows the public API to remain stable while the implementation can evolve.

### Singleton Pattern

- **Logger**: Centralized logging system accessed through `Logger::getInstance()`.

## State Machine

The measurement process is orchestrated by a state machine implemented in `MeasurementManagerImpl`. The state machine handles initialization, measurement requests, data fetching, and error recovery.

### State Flow

The state machine initializes the system and then runs a recurring, tick-based measurement loop:

1. **Initialization**: Reset boards, synchronize lights, configure interfaces and devices, then prepare the scheduler.
2. **Measurement ticks**: Wait for pending operations, trigger devices due for measurement, fetch completed results, execute queued device actions, and advance to the next tick.
3. **Recovery**: Handle measurement or communication errors and attempt recovery when enabled by `ManagerParams::repair_errors`.

The state machine can run either:
- In an internal thread via `startMeasuring()` / `stopMeasuring()`
- In the user's thread via repeated calls to `measureSome()`

## Communication Architecture

The library uses a communication interface abstraction to support multiple transport protocols:

- **SocketCAN**: Linux CAN FD interface
- **USBtingo**: USB-to-CAN converter interface

### Communication Flow

Messages flow through the communication layer using endpoint-filtered subscriptions:

1. `ComInterface` receives messages from hardware
2. Registered callbacks for the destination endpoint are notified
3. Board or device code processes the command and payload

### Endpoint System

Communication messages carry endpoints used to route requests and responses between the host, boards, and devices.

## Sensor Data Processing

### Measurement Flow

1. **Request**: MeasurementManager requests measurements from SensorRing
2. **Command**: SensorRing sends commands via SensorBus to SensorBoard
3. **Hardware**: Sensor boards execute measurements
4. **Receive**: Boards and devices receive data through communication subscriptions
5. **Process**: Sensors parse and transform measurement data
6. **Notify**: MeasurementManager publishes to subscribed callbacks

### Coordinate Transformation

Sensors support pose-based placement. Device poses combine the board pose with
the device's pose offset, using `Vector3` translation and Euler-angle rotation
values.

## Threading Model

`startMeasuring()` runs the state machine on a dedicated worker thread.
Alternatively, the application can drive it by repeatedly calling
`measureSome()`; stop the worker before using this mode. Subscriptions are RAII
handles and should remain alive for as long as their callbacks are needed.

## Key Implementation Details

### Sensor Board Management

Board expectations and device configuration are declared through
`SensorRingFactory::expectBoard()`, `expectDevice()`, and
`setDefaultDeviceParams()`. The factory matches these settings against the
enumerated board and device types.

### Parameter Cascading

Parameters are supplied to the component they configure: `ManagerParams` to
the `MeasurementManager`, communication-interface parameters to
`SensorRingFactory::addInterface()`, board parameters to `expectBoard()`, and
device parameters to `expectDevice()` or `setDefaultDeviceParams()`.

### Error Handling

The manager reports its health through `ManagerState`. `ManagerParams::timeout`
sets the timeout for sensor operations; `repair_errors` controls whether the
manager attempts recovery or shuts down on an error.

## Getting Started

### For New Developers

1. **Start with the public API**: Understand `MeasurementManager`, `SensorRingFactory`, and the subscription mechanism
2. **Study the scheduler**: Review `src/manager/MeasurementManagerImpl.cpp` and `MeasurementManagerImpl.hpp` to understand measurement phases.
3. **Explore device implementations**: Look under `src/device/` and the corresponding public headers under `include/sensorring/device/`.
4. **Review communication**: Explore `src/interface/` and the factory wiring when adding a transport.

### Common Extension Points

- **New Communication Interface**: Add a transport implementation under `src/interface/` and connect it through the factory.
- **New Sensor Type**: Implement a `Device` and the appropriate typed interface (`DepthSensor`, `ThermalSensor`, or `Light`), then wire discovery and construction through the factory.
- **New Board Type**: Add the board type and enumeration/configuration handling in `board/` and `factory/`.
- **Custom Processing**: Subscribe to a typed device group on `MeasurementManager` and retain the returned `Subscription`.

<div class="section_buttons"> 

| Read Previous | |
|:--|--:|
| [Software](03_software.md) | |

</div>
