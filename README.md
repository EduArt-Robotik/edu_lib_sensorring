# Sensor Ring Library

💡 **The EduArt Sensor Ring library is the high level software interface of the EduArt sensor boards.**

<div align="center">

<table  class="markdownTable">
    <tr class="markdownTableRowOdd">
        <td>Source Code</td>
        <td>
            <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring"><img src="https://img.shields.io/badge/edu_lib_sensorring-repo-blue?logo=github" alt="Source Code"></a>
            <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/releases/latest"><img src="https://img.shields.io/github/v/release/EduArt-Robotik/edu_lib_sensorring" alt="Latest release"></a>
        </td>
    </tr>
    <tr class="markdownTableRowEven">
        <td class="markdownTableBodyNone">
            License
        </td>
        <td class="markdownTableBodyNone">
            <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/LICENSE"><img src="https://img.shields.io/badge/License-BSD--3--Clause-blue?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH5gQFFyMP+ejbyAAAAVRJREFUOMuNkk0rhGEUhq+jETaajSxEKdvJShayRUlZWbCYjZr/4N+oyVjLwkdWkoWl2ShSllKmZKGZuiwcenuHyV2nzsd9n+e5ex4YAHVb3R7EqfwhnAM6wFbWZ0A1Iu7L3CiIloEVYAG4BWaBpRxfAY9ADbgBziLisnyDSaAHbEREV60CTznbjYiOOgzsJbfPwhvwEhHdrN+BtUJOLn5Jbp/vQ/W8UFfV54xqoX+uHn7XQ9mcALrAtbqYp3WAC+Aic3J2DXRT87UA2AEOgH2gPuDV6sk5SM3PghXgNCIegCl17BeLY8BUck5Tw5C6CbwCNXUeaAMNdaQgHgEaQDs5NeBV3UQ9sh9ttaK2MirZK+MIdVxtqh9qTz1Rp/PkltrKfDpnveQ21fGix2F1tOT7Z0GhN5ofajDUdbWTsc5/oc6oq+pdwetd9mb+s+DYv3Fc5n8Cd+5Qbrzh2X0AAAAASUVORK5CYII=" alt="License"></a>
        </td>
    </tr>
</table>

</div>

## 1 Quick Start Guide

🔧 **Choose your target platform or target framework for a quick start guide.**

<div align="center">

<table  class="markdownTable">
    <tr class="markdownTableRowOdd">
        <td class="markdownTableBodyNone">
            Target Platform
        </td>
        <td class="markdownTableBodyNone">
            <a href="doc/pages/installation.md"><img src="https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white" alt="Windows"></a>
            <a href="doc/pages/installation.md"><img src="https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black" alt="Linux"></a>
        </td>
    </tr>
    <tr class="markdownTableRowEven">
        <td class="markdownTableBodyNone">
            Target Framework
        </td>
        <td class="markdownTableBodyNone">
            <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp"><img src="https://img.shields.io/badge/C++-00599C?logo=cplusplus&logoColor=white" alt="C++"></a>
            <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/cpp"><img src="https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=white" alt="Python"></a>
            <a href="https://github.com/EduArt-Robotik/edu_sensorring_ros1"><img src="https://img.shields.io/badge/ROS1-22314E?logo=ros&logoColor=white" alt="ROS"></a>
            <a href="https://github.com/EduArt-Robotik/edu_sensorring_ros2"><img src="https://img.shields.io/badge/ROS2-22314E?logo=ros&logoColor=white" alt="ROS2"></a>
        </td>
    </tr>
</table>

</div>

## 2 Application Example

<div align="center">

<img src="doc/images/bot_front.png" width=600 onerror="this.onerror=null; this.src='bot_front.png';"><br>
A Raspberry Pi based EduBot equipped 12 sensor boards.


<img src="doc/images/bot_front_close.jpg" width=600 onerror="this.onerror=null; this.src='bot_front_close.jpg';"><br>
Close-up view of the three front facing sensors of the EduBot.


<img src="doc/images/map.png" width=600 onerror="this.onerror=null; this.src='map.png';"><br>
3D map of an office room recorded with the EduBot and <a href="https://octomap.github.io/">Octomap.</a>

</div>

## 3 Supported Hardware

> ℹ️ Have a look at the [hardware guide](doc/pages/01_hardware.md) on how to set up and use the sensors with your computer.

The supported sensors of the Sensor Ring library are listed in the following table:

<div align="center">

| <p align="center">Sensor Board Type</p> | <p align="center">Description</p> |
|---|---|
|  <p align="center">Edu Headlight</p> <p align="center">  <img src="doc/images/EDU_Headlight.png" width="200" onerror="this.onerror=null; this.src='EDU_Headlight.png';"></p> | <p align="left">- Headlight of the Raspberry Pi based EduBot <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - Optional Heimann HTPA32 thermal sensor <br> - 11 Addressable RGB Leds <br> - CAN FD Interface <br> - Input voltage range 6 V - 65 V DC</p>|
|  <p align="center">Edu Taillight</p> <p align="center">  <img src="doc/images/EDU_Taillight.png" width="200" onerror="this.onerror=null; this.src='EDU_Taillight.png';"></p> | - Taillight of the Raspberry Pi based EduBot <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - 2 Addressable RGB Leds <br> - CAN FD Interface<br> - Input voltage range 6 V - 65 V DC |
|  <p align="center">Edu Sidepanel</p> <p align="center">  <img src="doc/images/EDU_Sidepanel.png" width="200" onerror="this.onerror=null; this.src='EDU_Sidepanel.png';"></p> | - General purpose sensor board <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - 2 Addressable RGB Leds <br> - 38 × 28 mm <br> - CAN FD Interface<br> - Input voltage range 6 V - 65 V DC|
|  <p align="center">Edu Minipanel</p> <p align="center">  <img src="doc/images/EDU_Minipanel.png" width="200" onerror="this.onerror=null; this.src='EDU_Minipanel.png';"></p> | - General purpose sensor board <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - 28 × 24 mm  <br> - CAN FD Interface<br> - Input voltage range 6 V - 65 V DC|

</div>

## 4 Installation

> ℹ️ Have a look at the [installation guide](doc/pages/02_instatllationö.md) on how build the library on your platform.

**Quick start - Building the library:**
<div class="tabbed">

- <b class="tab-title">**Linux**</b><div class="darkmode_inverted_image">

    ```sh
    git clone https://github.com/EduArt-Robotik/edu_lib_sensorring
    mkdir -p edu_lib_sensorring/build
    cd edu_lib_sensorring/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
    cmake --build . -j4
    sudo cmake --install
    ```
  </div>

- <b class="tab-title">**Windows**</b><div class="darkmode_inverted_image">

    ```powershell
    git clone https://github.com/EduArt-Robotik/edu_lib_sensorring
    mkdir edu_lib_sensorring/build
    cd edu_lib_sensorring/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
    cmake --build . --config=Release -- -j4
    cmake --install . --config=Release # Requires terminal with admin privileges
    ```
</div>
</div>


## 5 Software

> ℹ️ Have a look at the [software guide](doc/pages/03_software.md) on how to use the library in your own projects.


```cpp
/**
 * Minimal example
 */
```

## 6 Developer documentation

> ℹ️ Refer to the [developer documentation](doc/pages/04_dev_doc.md) for more details on how the library is implemented.


<div class="section_buttons"> 

| | Read Next |
|:--|--:|
| | [Hardware](doc/pages/01_hardware.md) |

</div>