# Hardware

The Sensorring library currently only implements the Linux SocketCAN interface for communication with the Sensors. Therefore the recommended setup is a Linux environment with a CAN FD adapter. Possible hardware configurations are:

- A Raspberry Pi with an EduArt CAN FD to SPI shield. The shield provides two independent CAN FD interfaces that can be used for the sensors. Refer to [this guide](https://github.com/EduArt-Robotik/edu_robot/blob/main/documentation/setup/raspberry/setup_raspberry.md) on how to set up a Raspberry Pi with an EduArt Can shield.
- A generic Linux machine with a CAN FD to USB converter. In this case an additional external power supply is needed to power the sensors. Refer to the manual of your CAN FD to USB converter for a installation guide.

After setting up your environment check if one or more CAN interfaces are active with `ifconfing`. The output should show at least one running CAN interface.

```
eduart-can0: flags=193<UP,RUNNING,NOARP>  mtu 72
unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
RX packets 0  bytes 0 (0.0 B)
RX errors 0  dropped 0  overruns 0  frame 0
TX packets 0  bytes 0 (0.0 B)
TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
device interrupt 170
```

<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Readme](../../README.md) | [Installation](02_installation.md) |

</div>