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

<img src=../images/communication.png width=1000 onerror="this.onerror=null; this.src='communication.png';">



<div class="section_buttons"> 

| Read Previous | |
|:--|--:|
| [Software](03_software.md) | |

</div>