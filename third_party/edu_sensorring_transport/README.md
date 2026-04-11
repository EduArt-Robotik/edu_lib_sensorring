# EduArt Sensor Ring Transport

> Transport protocol library for the EduArt Sensor Ring — shared between the host-side C++ library and microcontroller firmware.

[![License](https://img.shields.io/badge/License-BSD--3--Clause-blue.svg)](LICENSE)

## Overview

This library implements the transport layer of the EduArt Sensor Ring protocol. It provides:

- **Protocol definitions** — frame structure, device IDs, commands, and fragment types
- **Message assembly** — splits outgoing messages into transport-sized frames
- **Message reassembly** — reconstructs complete messages from incoming fragments
- **CAN FD codec** — encodes/decodes `TransportFrame` to/from CAN FD wire format

The transport layer is **transport-agnostic** at its core. The `TransportFrame` structure and assembly/reassembly logic are independent of any specific bus. Transport-specific codecs (currently CAN FD) map between `TransportFrame` and the wire format.

## Requirements

- C++17 compiler
- CMake ≥ 3.13

## Integration

### As a git submodule

```sh
git submodule add https://github.com/EduArt-Robotik/edu_sensorring_transport.git third_party/edu_sensorring_transport
```

```cmake
add_subdirectory(third_party/edu_sensorring_transport)
target_link_libraries(your_target PRIVATE sensorring_transport::sensorring_transport)
```

### Via FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
  edu_sensorring_transport
  GIT_REPOSITORY https://github.com/EduArt-Robotik/edu_sensorring_transport.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(edu_sensorring_transport)
target_link_libraries(your_target PRIVATE sensorring_transport::sensorring_transport)
```

## Usage

```cpp
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/MessageAssembler.hpp>
#include <sensorring_transport/MessageReassembler.hpp>
#include <sensorring_transport/can/CanCodec.hpp>

using namespace sensorring::transport;
using namespace sensorring::transport::can;
using namespace eduart::transport::protocol;

// Assemble a message into CAN FD frames
MessageAssembler assembler(CanCodec::MAX_PAYLOAD_PER_FRAME);
auto frames = assembler.assemble(
    Direction::Output, 0x01, devbyte::VL53L8CX,
    vl53l8cx::MEASUREMENT_REQUEST, payload);

for (const auto& frame : frames) {
    auto canFrame = CanCodec::encode(frame);
    // send canFrame.id and canFrame.data over CAN bus
}

// Reassemble incoming CAN FD frames
MessageReassembler reassembler([](const TransportFrame& frame) {
    // complete message delivered here
});

// In your CAN receive handler:
auto transportFrame = CanCodec::decode(canId, data, dataLen);
reassembler.processFrame(transportFrame);
```

## Building Tests

Tests are built automatically when this is the top-level project:

```sh
mkdir build && cd build
cmake ..
cmake --build .
./sensorring_transport_tests
```

When embedded in a parent project, set `SENSORRING_TRANSPORT_BUILD_TESTS=ON` to include transport tests.

## License

BSD 3-Clause — see [LICENSE](LICENSE).
