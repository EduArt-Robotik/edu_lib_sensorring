# Sensorring Library
Software interface of the EduArt Sensorring.

## 1. Hardware
The sensorring library supports a variety of EduArt sensor boards. See the table below for the currently supported hardware:


|  <p align="center">Edu Headlight</p> <p align="center">  <img src="doc/images/headlight.png" height="180"/></p> | <p align="left">- Headlight of the Raspberry Pi based EduBot <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> -Optional Heimann HTPA32 thermal sensor <br> - 11 RGB Leds <br> - CANFD Interface </p>|
|---|---|
|  <p align="center">Edu Taillight</p> <p align="center">  <img src="doc/images/taillight.png" height="180"/></p> | - Taillight of the Raspberry Pi based EduBot <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - 2 RGB Leds <br> - CANFD Interface |
|  <p align="center">Edu Sidepanel</p> <p align="center">  <img src="doc/images/sidepanel.png" height="180"/></p> | - General Purpose sensor board <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - 2 RGB Leds <br> -38 × 28 mm <br> - CANFD Interface|
|  <p align="center">Edu Minipanel</p> | - General Purpose sensor board <br> - ST VL53L8CX 8 × 8 Time-of-Flight sensor <br> - 28 × 24 mm  <br> - CANFD Interface|


## 1. Sensor topology

The EduArt Sensorring is a sensor system that collects and combines measurements from multiple individual sensors. The individual sensors are daisy chained together in series and share a communication interface and a power supply. The chain of sensor is terminated by a master device at one end e.g. a Raspberry Pi or a CAN to USB converter. It is possible to use multiple communication interfaces to distribute the sensor data and enable the use of more sensors simultaneously. The following class diagram illustrates the topology and the reflection of the hardware layers in the library.

<img src="doc/images/class_diagram_simple.png" width="800"/>

## 2. Input Parameters

Each of the core classes of the library has its own parameter set which is shown in the above class diagram. The parameters have to be initialized externally and passed to the MeasurementManager upon initialization. The individual parameters are described [here](doc/parameters.md). See section _3. Software interface of the library_ for details on how the parameter structures are implemented.

## 3. Software interface of the library

See [here](doc/sw_interface.md) for a description of the software interface of the library.

## 4. Building and installing the library

See [here](doc/build_guide.md) for a guide on how to build and install the library.

## 5. Using the library in a custom project

Please refer to to the [Ros](https://github.com/EduArt-Robotik/edu_sensorring_ros1) and [Ros2](https://github.com/EduArt-Robotik/edu_sensorring_ros2) wrappers of this library for examples on how to use the library in your own project.