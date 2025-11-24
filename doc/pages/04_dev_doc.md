# Developer Documentation

## State machine

Part of the Sensorring library is a state machine that handles the timing of all actions.
It is implemented in the class [MeasurementManager](https://github.com/EduArt-Robotik/edu_lib_sensorring/blob/master/src/MeasurementManager.cpp) class.
The class either runs an internal thread that processes the state machine or the state machine is run in an external process with repeated calls of the `measureSome()` method.
The internal thread processes the state machine at a hard coded frequency of 2000 Hz.


The state machine starts with an initialization part that is executed once and then enters a loop.
Measurements are repeatedly requested in the loop and processed as they are received.
Note that the Time-of-Flight measurements have a higher priority than the thermal measurements.
If a frequency is specified for the Time-of-Flight sensors, the loop is throttled to match that frequency as close as possible.
The thermal measurements are fitted into the time frame of the Time-of-Flight sensor measurements when their measurement period is exceeded.
The result is that in most cases the thermal measurement frequency will be lower than the specified frequency.

Below is a flowchart illustrating the state machine operation.

<img src=../images/state_machine.png width=800 onerror="this.onerror=null; this.src='state_machine.png';">

## Communication interfaces

The library has a generic communication layer that abstracts the actual communication interfaces.
This allows the use of multiple interfaces and simplifies the addition of new interfaces.
All currently implemented interfaces are listed below.

<img src=../images/communication.png width=1000 onerror="this.onerror=null; this.src='communication.png';">

### SocketCAN
The library has implemented the Linux specific SocketCAN interface to communicate with the Sensorring.
To use this interface type make sure you have a CAN interface set up on your machine.
Use the `ifconfig` or `ip addr` command to verify that at least one CAN interface is up and running on your machine.
The SocketCAN interface can be used on most Linux machines by using a CAN FD adapter with Linux Kernel support like the [candleLightFD](https://linux-automation.com/de/products/candlelight-fd.html) adapter.

### USBtingo
The library has implemented a communication interface to the USBtingo CAN FD to USB adapter.
The USBtingo can be used from any operating system including Linux and Windows.
To use this interface the library [libusbtingo](https://github.com/hannesduske/libusbtingo) for this adapter needs to be installed.


<div class="section_buttons"> 

| Read Previous | |
|:--|--:|
| [Software](03_software.md) | |

</div>