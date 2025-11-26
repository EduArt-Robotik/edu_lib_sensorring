# Installation


## 1 Building the library from source

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


## 2 Installing the library

You can either install the library directly with CMake or generate installable packages.

### 2.1 Installing the library directly

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">

    ```sh
    sudo cmake --install .
    ```
    > ⚠️ The default installation directory is /usr/local. If another location is chosen, add your custom install path to the `CMAKE_PREFIX_PATH` environment variable for other CMake packages to find the library. Alternatively you can set `sensorring_DIR` if you don't want to manipulate `CMAKE_PREFIX_PATH`.
    
    ```sh
    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:<install-path>
    ```

  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">

    ```powershell
    cmake --install . # requires terminal with admin privileges
    ```
    > ⚠️ The default installation directory is C:\Program Files. If another location is chosen, add your custom install path to the `CMAKE_PREFIX_PATH` environment variable for other CMake packages to find the library. Alternatively you can set `sensorring_DIR` if you don't want to manipulate `CMAKE_PREFIX_PATH`.

    ```powershell
    $env:CMAKE_PREFIX_PATH = "$($env:CMAKE_PREFIX_PATH);<install-path>"
    ```

</div>
</div>


### 2.2 Generating an installable package

<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">

    ```sh
    cpack .
    ```
    > ℹ️ Running the command on Linux generates a .tar.gz archive and a .deb package. The .tar.gz can be extracted anywhere on the system with `tar -xvzf <package-name>.tar.gz -C /path/to/directory`. The .deb package can be installed with `dpkg -i <package name.deb>` which will install it in the `CMAKE_INSTALL_PREFIX` directory.

  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">

    ```powershell
    cpack .
    ```
    > ℹ️ Running the command on Windows generates .zip archive. The .zip archive can be be extracted anywhere on the system.


</div>
</div>

<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Hardware](01_hardware.md) | [Software](03_software.md) |

</div>
