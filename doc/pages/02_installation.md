# Installation

> 💡 You can either include the EduArt Sensor Ring library in your own CMake project via the [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) feature or build the library from source.

## 1 Using the Library in CMake Projects

The Sensor Ring library is easy to integrate in your own CMake projects.
Simply add these lines to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
  sensorring
  URL      https://github.com/EduArt-Robotik/edu_lib_sensorring/archive/refs/heads/master.zip
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
)

FetchContent_MakeAvailable(sensorring)
add_library(sensorring::sensorring ALIAS sensorring)
```

Here is an absolute minimum example of a `CMakeLists.txt` that includes the Sensor Ring library.

```cmake
cmake_minimum_required(VERSION 3.24)
project(minimal_example)

include(FetchContent)
FetchContent_Declare(
  sensorring
  URL      https://github.com/EduArt-Robotik/edu_lib_sensorring/archive/refs/heads/master.zip
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
)
 
FetchContent_MakeAvailable(sensorring)
add_library(sensorring::sensorring ALIAS sensorring)

add_executable(minimal_example
  src/main.cpp
)

target_link_libraries(minimal_example
  sensorring::sensorring
)
```

You can then simply include the Sensor Ring headers into your C++ program and compile the project.

```cpp
// Example of a C++ program using the library

#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/interface/InterfaceParams.hpp>
#include <sensorring/manager/MeasurementManager.hpp>

using namespace eduart::sensorring;

int main(int, char*[]) {
  com::UsbTingoParams interface{ "0" };
  SensorRingFactory factory;
  factory.addInterface(interface);
  manager::ManagerParams params;

  manager::MeasurementManager manager(params, factory);

  // Do something useful here

  return 0;
}
```

## Dependency Resolution

The transport dependency, the bootloader dependency (when
`SENSORRING_BUILD_FIRMWARE_UPDATE=ON`), libusbtingo (when
`SENSORRING_USE_USBTINGO=ON`), and Catch2 (when `SENSORRING_BUILD_TESTS=ON`)
are automatically resolved by CMake in this order:

1. Use the local checkout/submodule in `external/`.
2. Try an installed CMake config package.
3. Fetch the dependency from GitHub.

| Dependency | Local checkout | CMake config package | GitHub fallback |
|:--|:--|:--|:--|
| Transport | `external/edu_sensorring_transport` | `sensorring_transport` | `master` branch |
| Bootloader | `external/frankly_bootloader` | `frankly_bootloader` or `frankly-bootloader` | `master` branch |
| libusbtingo | `external/libusbtingo` | `usbtingo` | `v1.1.4` |
| Catch2 | `external/Catch2` | `Catch2` (version 3) | `v3.5.2` |

## 2 Building the Library from Source

### 2.1 Building the library

The library is built with a standard CMake workflow which is almost identical for Windows and Linux.

> ℹ️ Use `--recursive` when cloning to include the dependencies as submodules.
> If you already cloned without it, run `git submodule update --init --recursive`.
> Missing submodules are handled automatically as described in [Dependency Resolution](#dependency-resolution).

Use the following commands to build the library.

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">

    ```sh
    git clone --recursive https://github.com/EduArt-Robotik/edu_lib_sensorring
    mkdir -p edu_lib_sensorring/build
    cd edu_lib_sensorring/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DSENSORRING_BUILD_EXAMPLES=ON
    cmake --build . -j4
    ```

    **Linux Build Options:**

    <table>
      <tr><th>Build Option</th><th>Default Value</th><th>Description</th></tr>
      <tr><td>SENSORRING_BUILD_DOCUMENTATION</td><td>OFF</td><td>Build the documentation</td></tr>
      <tr><td>SENSORRING_BUILD_EXAMPLES</td><td>OFF</td><td>Build the example programs</td></tr>
      <tr><td>SENSORRING_BUILD_LIB</td><td>ON</td><td>Build the main sensor ring library</td></tr>
      <tr><td>SENSORRING_BUILD_UTILS</td><td>OFF</td><td>Build utility command line programs (requires firmware update module)</td></tr>
      <tr><td>SENSORRING_BUILD_FIRMWARE_UPDATE</td><td>OFF</td><td>Build the firmware update module</td></tr>
      <tr><td>SENSORRING_BUILD_PYTHON_BINDINGS</td><td>OFF</td><td>Build Python bindings</td></tr>
      <tr><td>SENSORRING_BUILD_SHARED_LIBS</td><td>OFF</td><td>Build as a shared library; OFF builds a static library</td></tr>
      <tr><td>SENSORRING_USE_SOCKETCAN</td><td>ON</td><td>Compile with support for Linux SocketCAN</td></tr>
      <tr><td>SENSORRING_USE_USBTINGO</td><td>ON</td><td>Compile with support for the USBtingo USB adapter</td></tr>
      <tr><td>CMAKE_BUILD_TYPE</td><td>Release</td><td>Choose the type of build (Debug/Release/RelWithDebInfo)</td></tr>
      <tr><td>CMAKE_INSTALL_PREFIX</td><td>/usr/local</td><td>Install path prefix, prepended onto install directories</td></tr>
    </table>

  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">

    ```powershell
    git clone --recursive https://github.com/EduArt-Robotik/edu_lib_sensorring
    mkdir edu_lib_sensorring/build
    cd edu_lib_sensorring/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DSENSORRING_BUILD_EXAMPLES=ON
    cmake --build . --config=Release --parallel
    ```

    **Windows build options:**

    <table>
      <tr><th>Build Option</th><th>Default Value</th><th>Description</th></tr>
      <tr><td>SENSORRING_BUILD_DOCUMENTATION</td><td>OFF</td><td>Build the documentation</td></tr>
      <tr><td>SENSORRING_BUILD_EXAMPLES</td><td>OFF</td><td>Build the example programs</td></tr>
      <tr><td>SENSORRING_BUILD_LIB</td><td>ON</td><td>Build the main sensor ring library</td></tr>
      <tr><td>SENSORRING_BUILD_UTILS</td><td>OFF</td><td>Build utility command line programs (requires firmware update module)</td></tr>
      <tr><td>SENSORRING_BUILD_FIRMWARE_UPDATE</td><td>OFF</td><td>Build the firmware update module</td></tr>
      <tr><td>SENSORRING_BUILD_PYTHON_BINDINGS</td><td>OFF</td><td>Build Python bindings</td></tr>
      <tr><td>SENSORRING_BUILD_SHARED_LIBS</td><td>OFF</td><td>Build as a shared library; OFF builds a static library</td></tr>
      <tr><td>SENSORRING_USE_USBTINGO</td><td>ON</td><td>Compile with support for the USBtingo USB adapter</td></tr>
      <tr><td>CMAKE_BUILD_TYPE</td><td>Release</td><td>Choose the type of build (Debug/Release/RelWithDebInfo)</td></tr>
      <tr><td>CMAKE_INSTALL_PREFIX</td><td>C:\\Program Files</td><td>Install path prefix, prepended onto install directories</td></tr>
    </table>

</div>
</div>


### Building only the utilities

The discovery tool and firmware updater can be built without the main sensor
library or sensor drivers:

```sh
cmake -S . -B build/utils -DCMAKE_BUILD_TYPE=Release \
  -DSENSORRING_BUILD_LIB=OFF \
  -DSENSORRING_BUILD_UTILS=ON \
  -DSENSORRING_BUILD_FIRMWARE_UPDATE=ON
cmake --build build/utils --parallel
cmake --install build/utils
```

Alternatively, use `cmake --preset ReleaseUtils`, `cmake --build --preset ReleaseUtils`
and `ctest --preset ReleaseUtils` to also build and run the base-layer unit tests.
USBtingo support can be disabled with `-DSENSORRING_USE_USBTINGO=OFF` on Linux.

`SENSORRING_BUILD_LIB` defaults to `ON`. When it is `OFF`, examples, Python
bindings and hardware tests must also be `OFF`; `SENSORRING_BUILD_TESTS=ON`
builds only the base-layer tests. The firmware module may also be built
independently, without the utilities or main library.

The shared communication/discovery target is `sensorring::base`; firmware
updating uses `sensorring::firmware_update`. The main library target,
`sensorring::sensorring`, is only available when `SENSORRING_BUILD_LIB=ON`.
Existing library discovery/reset APIs delegate to the shared base layer.
Discovery itself never constructs or configures sensor devices.

With `SENSORRING_INSTALL=ON`, installation includes the enabled libraries,
their public headers, CMake targets and utility executables. With it `OFF`,
these installation rules are omitted. Shared builds require the base and
firmware shared libraries at runtime as well as any enabled USBtingo dependency.

### 2.2 Installing the Library

After building the library you can install it to make it available to all projects on your computer.

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">

    ```sh
    sudo cmake --install .
    ```
    > ⚠️ The default installation directory of the library is `/usr/local`. If another location is chosen, add your custom install path to the `CMAKE_PREFIX_PATH` environment variable for other CMake packages to find the library. Alternatively you can set `sensorring_DIR` if you don't want to manipulate `CMAKE_PREFIX_PATH`.
    ```sh
    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:<install-path>
    ```

  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">

    ```powershell
    cmake --install . # requires terminal with admin privileges
    ```
    > ⚠️ The default installation directory of the library is `C:\Program Files`. If another location is chosen, add your custom install path to the `CMAKE_PREFIX_PATH` environment variable for other CMake packages to find the library. Alternatively you can set `sensorring_DIR` if you don't want to manipulate `CMAKE_PREFIX_PATH`.
    ```powershell
    $env:CMAKE_PREFIX_PATH = "$($env:CMAKE_PREFIX_PATH);<install-path>"
    ```

</div>
</div>


### 2.3 Generating Installable Packages

Instead of installing the library directly, you can generate installable packages to distribute it to other computers.
However, distributing the installable package requires that the computer on which you build the library and the computers on which you install it have matching systems.

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">
    The installable Linux packages of the library can be generated with the following command:

    ```sh
    cpack .
    ```
    > ℹ️ Running the command on Linux generates a `.tar.gz` archive and a `.deb` package.
    Generated packages are located in the directories `edu_lib_sensorring/release` or `edu_lib_sensorring/debug` depending on the build type.
    
    The `.tar.gz` archive can be extracted anywhere on the system with the following command:
    
    ```sh
    tar -xvzf ../release/sensorring-<version>-<arch>.tar.gz -C /path/to/directory
    ```
    
    The `.deb` package can be installed with the following command which will install the library in the `CMAKE_INSTALL_PREFIX` directory:
    
    ```sh
    sudo dpkg -i ../release/sensorring-<version>-<arch>.deb
    ```
    > ⚠️ The default installation directory of the library is `/usr/local`. If another location is chosen, add your custom install path to the `CMAKE_PREFIX_PATH` environment variable for other CMake packages to find the library. Alternatively you can set `sensorring_DIR` if you don't want to manipulate `CMAKE_PREFIX_PATH`.
    ```sh
    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:<install-path>
    ```

  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">
    The installable Windows packages of the library can be generated with the following command:

    ```powershell
    cpack .
    ```
    > ℹ️ Running the command on Windows generates `.zip archive`.
    Generated packages are located in the directories `edu_lib_sensorring/release` or `edu_lib_sensorring/debug` depending on the build type.

    The archive can be be extracted anywhere on the system using the Windows File Explorer.
    > ⚠️ The default installation directory of the library is `C:\Program Files`. If another location is chosen, add your custom install path to the `CMAKE_PREFIX_PATH` environment variable for other CMake packages to find the library. Alternatively you can set `sensorring_DIR` if you don't want to manipulate `CMAKE_PREFIX_PATH`.
    ```powershell
    $env:CMAKE_PREFIX_PATH = "$($env:CMAKE_PREFIX_PATH);<install-path>"
    ```

  </div>

</div>


<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Hardware](01_hardware.md) | [Software](03_software.md) |

</div>
