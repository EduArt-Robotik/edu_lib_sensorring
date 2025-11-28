# Sensor Ring Library

> 💡 **The EduArt Sensor Ring library is the high level software interface of the EduArt sensor boards.**

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

## Quick Start Guide

Choose your target platform or target framework for a quick start guide:

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
            <a href="https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/apps/examples/python"><img src="https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=white" alt="Python"></a>
            <a href="https://github.com/EduArt-Robotik/edu_sensorring_ros1"><img src="https://img.shields.io/badge/ROS1-22314E?logo=ros&logoColor=white" alt="ROS"></a>
            <a href="https://github.com/EduArt-Robotik/edu_sensorring_ros2"><img src="https://img.shields.io/badge/ROS2-22314E?logo=ros&logoColor=white" alt="ROS2"></a>
        </td>
    </tr>
</table>

</div>

## Application Example

<div align="center">
<table style="border: none;">
  <tr>
    <td style="text-align:center">
      <img src="doc/images/bot_front.png" height=300 onerror="this.onerror=null; this.src='bot_front.png';"><br>
      A Raspberry Pi based EduBot equipped 12 sensor boards.
    </td>
    <td style="text-align:center">
      <img src="doc/images/map.png" height=300 onerror="this.onerror=null; this.src='map.png';"><br>
      3D map of an office room recorded with the EduBot and <a href="https://octomap.github.io/">Octomap.</a>
    </td>
  </tr>
</table>
</div>

## Hardware

> ℹ️ Have a look at the [hardware guide](doc/pages/01_hardware.md) on how to set up and use the sensors with your computer.

## Installation

> ℹ️ Have a look at the [installation guide](doc/pages/02_installation.md) on how build the library on your platform.

### Quick Start - Use the library in other CMake projects

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


### Quick Start - Build and install the library from source

<div class="tabbed">

- <b class="tab-title">**Quick start Linux**</b><div class="darkmode_inverted_image">

  ```sh
  git clone https://github.com/EduArt-Robotik/edu_lib_sensorring
  mkdir -p edu_lib_sensorring/build
  cd edu_lib_sensorring/build
  cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
  cmake --build . -j4
  sudo cmake --install
  ```
</div>

- <b class="tab-title">**Quick start Windows**</b><div class="darkmode_inverted_image">

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



## Software

> ℹ️ Have a look at the [software guide](doc/pages/03_software.md) on how to use the library in your own projects.


<div class="section_buttons"> 

| | Read Next |
|:--|--:|
| | [Hardware](doc/pages/01_hardware.md) |

</div>