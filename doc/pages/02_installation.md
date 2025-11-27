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
cmake_minimum_required(VERSION 3.10)
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

You can then simply integrate the Sensor Ring headers into your C++ program and compile the project.

```cpp
// Example of a C++ program using the library

#include <sensorring/MeasurementManager.hpp>

using namespace eduart;

int main(int, char*[]) {

  manager::ManagerParams params;

  // Populate the parameters

  auto manager = std::make_unique<manager::MeasurementManager>(params);

  // Do something useful here

  return 0;
}
```

## 2 Building the Library from Source

### 2.1 Building the library

The library is built with a standard CMake workflow which is almost identical for Windows and Linux. Use the following commands to build the library.

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">

    ```sh
    git clone https://github.com/EduArt-Robotik/edu_lib_sensorring
    mkdir -p edu_lib_sensorring/build
    cd edu_lib_sensorring/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
    cmake --build . -j4
    ```

    **Linux Build Options:**

    <table>
      <tr><th>Build Option</th><th>Default Value</th><th>Description</th></tr>
      <tr><td>BUILD_DOCUMENTATION</td><td>ON</td><td>Build the documentation</td></tr>
      <tr><td>BUILD_EXAMPLES</td><td>ON</td><td>Build the example programs</td></tr>
      <tr><td>BUILD_PYTHON_BINDINGS</td><td>ON</td><td>Build python bindings</td></tr>
      <tr><td>BUILD_SHARED_LIBS</td><td>ON</td><td>Build as shared library</td></tr>
      <tr><td>USE_SOCKETCAN</td><td>ON</td><td>Compile with support for Linux SocketCAN</td></tr>
      <tr><td>USE_USBTINGO</td><td>ON</td><td>Compile with support for the USBtingo USB adapter</td></tr>
      <tr><td>CMAKE_BUILD_TYPE</td><td>Release</td><td>Choose the type of build (Debug/Release/RelWithDebInfo)</td></tr>
      <tr><td>CMAKE_INSTALL_PREFIX</td><td>/usr/local</td><td>Install path prefix, prepended onto install directories</td></tr>
    </table>

  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">

    ```powershell
    git clone https://github.com/EduArt-Robotik/edu_lib_sensorring
    mkdir edu_lib_sensorring/build
    cd edu_lib_sensorring/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
    cmake --build . --config=Release -- -j4
    ```

    **Windows build options:**

    <table>
      <tr><th>Build Option</th><th>Default Value</th><th>Description</th></tr>
      <tr><td>BUILD_DOCUMENTATION</td><td>ON</td><td>Build the documentation</td></tr>
      <tr><td>BUILD_EXAMPLES</td><td>ON</td><td>Build the example programs</td></tr>
      <tr><td>BUILD_PYTHON_BINDINGS</td><td>ON</td><td>Build python bindings</td></tr>
      <tr><td>BUILD_SHARED_LIBS</td><td>ON</td><td>Build as shared library</td></tr>
      <tr><td>USE_USBTINGO</td><td>ON</td><td>Compile with support for the USBtingo USB adapter</td></tr>
      <tr><td>CMAKE_BUILD_TYPE</td><td>Release</td><td>Choose the type of build (Debug/Release/RelWithDebInfo)</td></tr>
      <tr><td>CMAKE_INSTALL_PREFIX</td><td>C:\Program Files</td><td>Install path prefix, prepended onto install directories</td></tr>
    </table>

</div>
</div>


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




### 2.4 Uninstalling the Library


<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Hardware](01_hardware.md) | [Software](03_software.md) |

</div>
