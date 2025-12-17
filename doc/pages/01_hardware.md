# Hardware

## 1 EduArt Sensor Boards

The table below shows the current versions of the EduArt sensor boards that can be used with the EduArt Sensor Ring library.

<div align="center">
<table>
	<tr>
		<th style="text-align:center"><p>Sensor Board Type</p></th>
		<th style="text-align:center"><p>Description</p></th>
	</tr>
	<tr>
		<td style="text-align:center">
			<p><b>Edu Headlight</b></p>
			<p><img src="../images/EDU_Headlight.webp" width="200" onerror="this.onerror=null; this.src='EDU_Headlight.webp';"></p>
		</td>
		<td style="text-align:left">
			- Headlight of the Raspberry Pi based EduBot <br>
			- ST VL53L8CX 8 × 8 Time-of-Flight sensor <br>
			- Optional Heimann HTPA32x32d thermal sensor <br>
			- 11 Addressable RGB Leds <br>
			- CAN FD Interface <br>
			- Input voltage range 6 V - 65 V DC
		</td>
	</tr>
	<tr>
		<td style="text-align:center">
			<p><b>Edu Taillight</b></p>
			<p><img src="../images/EDU_Taillight.webp" width="200" onerror="this.onerror=null; this.src='EDU_Taillight.webp';"></p>
		</td>
		<td style="text-align:left">
			- Taillight of the Raspberry Pi based EduBot <br>
			- ST VL53L8CX 8 × 8 Time-of-Flight sensor <br>
			- 2 Addressable RGB Leds <br>
			- CAN FD Interface<br>
			- Input voltage range 6 V - 65 V DC
		</td>
	</tr>
	<tr>
		<td style="text-align:center">
			<p><b>Edu Sidepanel</b></p>
			<p><img src="../images/EDU_Sidepanel.webp" width="200" onerror="this.onerror=null; this.src='EDU_Sidepanel.webp';"></p>
		</td>
		<td style="text-align:left">
			- General purpose sensor board <br>
			- ST VL53L8CX 8 × 8 Time-of-Flight sensor <br>
			- 2 Addressable RGB Leds <br>
			- 38 × 28 mm <br>
			- CAN FD Interface<br>
			- Input voltage range 6 V - 65 V DC
		</td>
	</tr>
	<tr>
		<td style="text-align:center">
			<p><b>Edu Minipanel</b></p>
			<p><img src="../images/EDU_Minipanel.webp" width="200" onerror="this.onerror=null; this.src='EDU_Minipanel.webp';"></p>
		</td>
		<td style="text-align:left">
			- General purpose sensor board <br>
			- ST VL53L8CX 8 × 8 Time-of-Flight sensor <br>
			- 28 × 24 mm  <br>
			- CAN FD Interface<br>
			- Input voltage range 6 V - 65 V DC
		</td>
	</tr>
</table>
</div>


## 2 Main Components of the Sensor Boards

### 2.1 ST VL53L8CX Time of Flight sensor

All of the Edu Sensor Boards come with a ST VL53L8CX is a 3D multi zone Time of Flight sensor that returns a grid of 8 × 8 individual 3D distance values per measurement.
The measurements from all Time-of-Flight sensors in the EduArt Sensor Ring are combined in a single large point cloud whose resolution solely depends on the number of connected Time-of-Flight sensors.
<a href="https://www.st.com/en/imaging-and-photonics-solutions/vl53l8cx.html">[Product Website]</a>

The key features of the sensor according to the manufacturers [datasheet](https://www.st.com/resource/en/datasheet/vl53l8cx.pdf) are:

- Ranging up to 400 cm
- Measurements with 8 × 8 resolution at 15 Hz
- Square Field-of-View with 65° diagonal opening angle (45° vertical and horizontal)
- Accuracy of ±8% or better in most cases (refer to datasheet)

<div align="center">
<table style="border: none;">
	<tr>
		<td style="text-align:center">
			<img src="../images/vl53l8cx.webp" height=300 onerror="this.onerror=null; this.src='vl53l8cx.webp';"><br>
			The ST VL53L8CXTime-of-Flight sensor
		</td>
		<td style="text-align:center">
			<img src="../images/example_ros3.webp" height=300 onerror="this.onerror=null; this.src='example_ros3.webp';"><br>
			Visualization of a VL53L8CX measurement
		</td>
	</tr>
</table>
</div>

### 2.2 Heimann HTPA32x32d Thermophile Array

The Edu Headlight sensors boards can be delivered with an optional Heimann HTPA32x32d is an all-in-one integrated thermal imaging sensor.
It returns a thermal image with temperature values for each pixel.
In addition the Sensor Ring library converts the temperature image into a false color image for better visualization.
<a href="https://www.heimannsensor.com/32x32">[Product Website]</a>

The key features of the sensor according to the manufacturers [datasheet](https://cdn.website-editor.net/s/156d2965ff764637aaea150903bb0161/files/uploaded/Overview-Datasheet-HTPA_32x32d_Rev17..pdf?Expires=1766828974&Signature=ayPse7nhwOi4OYhHHa6741axk7JH3HkDCt~zpfZMiv6pI2rj3XoPFwJDAJweLorboxOXewXBjHCR59WJvzPGT~2e5WX2BuvGyUfPGh9jrswpmV50EMBWNfxNtui0V-iMOzQ6QjD3R-LqB5xKXP8lZzYJDB1rGqcShdnoX~wQBuVsr~jNtsRjvCfAlx4lyAZpBpHEwnJOkuhz-ypo9rExuUukEd-2pSWV55nhWJ0IbBE3urLdm-vdfxkbuYoDUiQCkNmIIuPWVxUj18UStx15E~7cHYqQn4W66fPocQcgeLt0~iFnX2jZTNg9l45IoMbIg6cAqt39CYmAFtrFN9z5-w__&Key-Pair-Id=K2NXBXLF010TJW) are:

- 32 × 32 pixel resolution
- Measures temperatures between -20°C and >1000°C
- Available with different optics
- Developed for person detection

<div align="center">
<table style="border: none;">
	<tr>
		<td style="text-align:center; padding: 20px;">
			<img src="../images/htpa32.webp" height=300 onerror="this.onerror=null; this.src='htpa32.webp';"><br>
			The Heimann HTPA32x32d Thermophile Arrays with different optics
		</td>
		<td style="text-align:center; padding: 20px;">
			<img src="../images/example_thermal1.webp" height=300 onerror="this.onerror=null; this.src='example_thermal1.webp';"><br>
			Visualization of a HTPA32x32d measurement
		</td>
	</tr>
</table>
</div>


### 2.3 Individually Addressable RGB LEDs

Some Sensor Boards come with individually addressable RGB LEDs.
The LEDs can either be set to predefined color animations or can be set individually through the library.
<a href="https://www.inolux-corp.com/details.php?i=230#SMD-LED">[Product Website]</a>

<div style="text-align:center">
<img src="../images/ws2812b.webp" height=300 onerror="this.onerror=null; this.src='ws2812b.webp';"><br>
Individually addressable RGB LEDs
</div>


## 3 Supported Communication Interfaces

### 3.1 SocketCAN <img src="https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black" alt="Linux">

The Sensor Ring library implements the Linux specific SocketCAN interface to communicate with the sensors.
The SocketCAN interface can be used on most Linux machines by using a CAN FD USB adapter with Linux Kernel support or through a SPI to CAN FD adapter for embedded computers like a Raspberry Pi.

To use the SocketCAN interface type you need to connect a compatible CAN adapter to your computer and start the interface.

```sh
sudo ip link set can0 up type can bitrate 1000000 dbitrate 5000000 fd on
```

> ⚠️ The EduArt sensors have a fixed baud rates of 1 Mbps nominal and 5 Mbps for the data. Make sure your interface matches these rates.

Use the `ifconfig` or `ip addr` command to verify that the CAN interfaces are up and running. In this example we have the CAN interface `can0` up and running.

```sh
$ ifconfig
can0: flags=193<UP,RUNNING,NOARP>  mtu 72
      unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
      RX packets 0  bytes 0 (0.0 B)
      RX errors 0  dropped 0  overruns 0  frame 0
      TX packets 0  bytes 0 (0.0 B)
      TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
      device interrupt 170
```


### 3.1.1 EduArt Raspberry Pi Adapter

EduArt provides a [RaspberryPi adapter shield](https://eduart-robotik.com/products/kinematicskit-kim/) that provides two independent CAN FD interfaces reserved for the sensors.
Refer to [this guide](https://github.com/EduArt-Robotik/edu_robot/blob/main/documentation/setup/raspberry/setup_raspberry.md) on how to set up a Raspberry Pi with the EduArt adapter shield.
After the setup of the shield you should see three running CAN interfaces with the `ifconfig` or `ip addr` command.

<div style="text-align:center">
<img src="../images/raspberry_top.webp" width=600 onerror="this.onerror=null; this.src='raspberry_top.webp';"><br>
A Raspberry Pi with the EduArt adapter shield and one minipanel sensor board.
</div>

### 3.1.2 SocketCAN USB Adapter

The SocketCAN interface can also be used on most Linux machines with a CAN FD adapter with Linux Kernel support like the [candleLightFD](https://linux-automation.com/de/products/candlelight-fd.html) adapter.
These adapters are super easy to use:

1. Connect the adapter via USB to your computer.
2. Run `sudo ip link set can0 up type can bitrate 1000000 dbitrate 5000000 fd on` to start the interface.
3. Verify that the interface is running with the `ifconfig` or `ip addr` command.
4. Optional: Create a UDEV rule that automatically starts the interface when the adapter is connected.

```sh
sudo bash -c 'echo "ACTION==\"add\", SUBSYSTEM==\"usb\", ATTR{idVendor}==\"1d50\", ATTR{idProduct}==\"606f\", TAG+=\"systemd\", ENV{SYSTEMD_WANTS}=\"candlelight-can0.service\"" > /etc/udev/rules.d/50-candlelight.rules'
sudo bash -c 'echo -e "#!/bin/bash\n\n# Wait for CAN interface to appear (up to 5 seconds)\nfor i in {1..10}; do\n    if ip link show can0 >/dev/null 2>&1; then\n        break\n    fi\n    sleep 0.5\ndone\n\n# Bring up CAN FD\n/usr/sbin/ip link set can0 up type can bitrate 1000000 dbitrate 5000000 fd on" > /usr/local/bin/start_can0.sh'
sudo chmod +x /usr/local/bin/start_can0.sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```

<div style="text-align:center">
<img src="../images/candlelight_top.webp" width=600 onerror="this.onerror=null; this.src='candlelight_top.webp';"><br>
A CandlightFD CAN FD to USB adapter with one minipanel sensor board.
</div>

### 3.2 USBtingo USB Adapter <img src="https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black" alt="Linux"> <img src="https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white" alt="Windows">

In addition to the Linux only SocketCAN interface the Sensor Ring library also implements the cross platform USBtingo CAN FD to USB interface.
The USBtingo can be used on both Linux and Windows devices.
The Sensor Ring Library uses [libusbtingo](https://github.com/hannesduske/libusbtingo) internally to support this interface.
If the libusbtingo library is not previously installed it is automatically fetched while building the the Sensor Ring library.

> ⚠️ The Sensor Ring library needs to be compiled with the `-DSENSORRING_USE_USBTINGO=ON` option to enable support for this interface.

> ⚠️ The USBtingo is not recognized as CAN device by the Linux Kernel. The commands from the SocketCAN compatible adapter above don't work for this device.

<div style="text-align:center">
<img src="../images/usbtingo_top.webp" width=600 onerror="this.onerror=null; this.src='usbtingo_top.webp';"><br>
A USBtingo CAN FD to USB adapter with one minipanel sensor board.
</div>

## 4 Connecting the Sensor Ring

The sensor boards of the Sensor Ring are daisy chained together with **Molex Pico-Lock 7-Pin** off-the-shelf cables.
The chain of sensors needs to be terminated at one one by a RaspberryPi with the EduArt adapter shield or by a computer with a CAN FD to USB adapter.

> ℹ️ The connectors on the boards have no dedicated input or output and can be joined in any order and orientation.

<div align="center">
<table style="border: none;">
<tr>
	<td style="text-align:center">
		<img src="../images/usbtingo_chain_top.webp" width=600 onerror="this.onerror=null; this.src='usbtingo_chain_top.webp';"><br>
		Top view of a sensor chain with two minipanels and an USBtingo as adapter.
	</td>
	<td style="text-align:center">
		<img src="../images/usbtingo_chain_bottom.webp" width=600 onerror="this.onerror=null; this.src='usbtingo_chain_bottom.webp';"><br>
		Bottom view of a sensor chain with two minipanels and an USBtingo as adapter.
	</td>
</tr>
</table>
</div>


<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Readme](../../README.md) | [Installation](02_installation.md) |

</div>