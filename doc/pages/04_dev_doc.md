# Developer Documentation

## Architecture Overview

The Sensor Ring library follows a layered architecture with clear separation between public API, internal implementation, and hardware abstraction layers. The system is organized hierarchically from high-level measurement management down to individual sensor hardware interfaces.

## Component Hierarchy

The library structure follows this hierarchy:

```
MeasurementManager (Public API)
    └── MeasurementManagerImpl (Implementation)
            └── SensorRing
                    └── SensorBus (one per communication interface)
                            └── SensorBoard (one per physical board)
                                    ├── TofSensor
                                    ├── ThermalSensor
                                    └── LedLight
```

### Component Responsibilities

- **MeasurementManager**: Public interface providing measurement control and client registration
- **MeasurementManagerImpl**: Internal implementation containing the state machine and measurement orchestration
- **SensorRing**: Top-level container managing multiple sensor buses
- **SensorBus**: Manages sensor boards on a single communication interface
- **SensorBoard**: Represents a physical sensor board containing ToF, thermal, and LED sensors
- **BaseSensor**: Base class for sensor implementations (TofSensor, ThermalSensor)

## Directory Structure

```
include/sensorring/          # Public API headers
    ├── MeasurementManager.hpp
    ├── MeasurementClient.hpp
    ├── Parameter.hpp
    └── types/               # Public type definitions

src/                         # Implementation files
    ├── MeasurementManager.cpp
    ├── MeasurementManagerImpl.cpp/hpp
    ├── SensorRing.cpp/hpp
    ├── SensorBus.cpp/hpp
    ├── SensorBoard.cpp/hpp
    ├── interface/          # Communication abstraction
    │   ├── ComInterface.hpp
    │   ├── ComManager.hpp
    │   └── can/            # CAN interface implementations
    ├── sensors/            # Sensor implementations
    │   ├── BaseSensor.hpp
    │   ├── TofSensor.cpp/hpp
    │   ├── ThermalSensor.cpp/hpp
    │   └── hardware/       # Hardware-specific code
    ├── boardmanager/       # Board configuration management
    ├── math/               # Math utilities
    └── types/              # Internal type definitions
```

## Design Patterns

### Observer Pattern

The library extensively uses the observer pattern for decoupled communication:

- **ComObserver**: Sensors and boards observe communication interfaces for incoming messages
- **MeasurementClient**: External clients observe the MeasurementManager for measurement data
- **LoggerClient**: Observers receive log messages from the Logger singleton

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

- **ComManager**: Manages communication interface instances
- **Logger**: Centralized logging system

## State Machine

The measurement process is orchestrated by a state machine implemented in `MeasurementManagerImpl`. The state machine handles initialization, measurement requests, data fetching, and error recovery.

### State Flow

The state machine operates in two phases:

1. **Initialization Phase** (executed once):
   - Reset sensors
   - Enumerate devices
   - Synchronize lights
   - Read EEPROM data

2. **Measurement Loop** (repeated):
   - Request ToF measurements (if enabled)
   - Fetch ToF data
   - Request thermal measurements (if enabled)
   - Fetch thermal data
   - Throttle to match configured frequencies
   - Handle errors and recovery

The state machine can run either:
- In an internal thread via `startMeasuring()` / `stopMeasuring()`
- In the user's thread via repeated calls to `measureSome()`

<div align="center">
<img src=../images/state_machine.webp width=1000 onerror="this.onerror=null; this.src='state_machine.webp';">
</div>

## Communication Architecture

The library uses an abstract communication interface (`ComInterface`) that allows multiple transport protocols. Currently supported:

- **SocketCANFD**: Linux SocketCAN interface
- **USBtingo**: USB-to-CAN converter interface

### Communication Flow

Messages flow through the communication layer using the observer pattern:

1. `ComInterface` receives messages from hardware
2. Registered `ComObserver` instances (sensors, boards) are notified
3. Observers process messages based on endpoint and payload

<div align="center">
<img src=../images/communication.webp width=1000 onerror="this.onerror=null; this.src='communication.webp';">
</div>

### Endpoint System

Communication uses an endpoint-based addressing system:
- **Broadcast**: Messages to all devices
- **Device-specific**: Messages to individual sensor boards
- **Status endpoints**: ToF and thermal status messages

## Sensor Data Processing

### Measurement Flow

1. **Request**: MeasurementManager requests measurements from SensorRing
2. **Command**: SensorRing sends commands via SensorBus to SensorBoard
3. **Hardware**: Sensor boards execute measurements
4. **Receive**: Sensors receive data via ComInterface observer callbacks
5. **Process**: Sensors parse and transform measurement data
6. **Notify**: MeasurementManager notifies registered MeasurementClients

### Coordinate Transformation

Sensors support pose-based coordinate transformation:
- Each sensor has a translation and rotation offset
- Measurements can be transformed from sensor frame to board frame
- Transformation uses `Matrix3` rotation matrices and `Vector3` translations

## Threading Model

The library uses multiple threads for concurrent operations:

- **Measurement Thread**: Runs the state machine (optional, if `startMeasuring()` is used)
- **Communication Listener Threads**: Each `ComInterface` runs a listener thread for incoming messages
- **Thread Safety**: Critical sections use mutexes (`std::mutex`) with lock guards

## Key Implementation Details

### Sensor Board Management

Sensor board configurations are managed by `SensorBoardManager`, which provides:
- Board type definitions (Sidepanel, Headlight, Taillight, Minipanel)
- Sensor specifications (ToF, thermal, LED configurations)
- Hardware-specific parameters

### Parameter Cascading

Parameters follow a cascading structure matching the system topology:
- `ManagerParams` → `RingParams` → `BusParams` → `SensorBoardParams` → `TofSensorParams` / `ThermalSensorParams`

### Error Handling

The state machine includes error handling states:
- `error_handler_measurement`: Handles measurement-related errors
- `error_handler_communication`: Handles communication failures
- Sensors maintain error states (`SensorState`) that are checked during operation

## Getting Started

### For New Developers

1. **Start with the public API**: Understand `MeasurementManager` and `MeasurementClient`
2. **Study the state machine**: Review `MeasurementManagerImpl::StateMachine()` to understand the measurement flow
3. **Explore sensor implementations**: Look at `TofSensor` and `ThermalSensor` to understand data processing
4. **Review communication layer**: Understand `ComInterface` and its implementations for adding new protocols

### Common Extension Points

- **New Communication Interface**: Implement `ComInterface` and register with `ComManager`
- **New Sensor Type**: Extend `BaseSensor` and add to `SensorBoard`
- **New Board Type**: Add configuration to `SensorBoardManager`
- **Custom Processing**: Implement `MeasurementClient` to process measurements

<div class="section_buttons"> 

| Read Previous | |
|:--|--:|
| [Software](03_software.md) | |

</div>
