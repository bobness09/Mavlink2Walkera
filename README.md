# Mavlink2Walkera
Arducopter/Pixhawk To Walkera Telemetry converter for Arduino

Author bobness09, Mail bobnessdev@gmail.com
Contact me, if you have any questions!

This project is a telemetry converter for using an Arducopter/Pixhawk with Walkeras telemetry-enabled Data-Port Receivers like the RX705 and up.
It is very easy to install, just connect the wires from Telemetry 2 Port of the Pixhawk to the Arduino, and one wire from the Arduino into the RX705 (telemetry port).
I successfully tested it with a Pixhawk 2.4.5, an RX705 and a DEVO 10 with Deviation firmware. Since the Mavlink protocol is the same in the Arducopter, it will work without problems, too.

Mavlink2Walkera is based on the following project. Thanks a lot, Nils Högberg!
https://github.com/vizual54/APM-Mavlink-to-FrSky

Things needed:
--------------
* Arducopter/Pixhawk, Walkera RX705 or higher, telemetry enabled Walkera transmitter

* Arduino Mini Pro (Clone) 328, 16MHz, 5V
* A few jumper cables
* Pixhawk/Arducopter Telemetry Port cable
* Walkera telemetry cable fitting into the telemetry port of the RX705
* Hex file (can be found in the Doc folder) and TTL to USB adapter to flash it. Here's how to flash the file: https://github.com/grbl/grbl/wiki/Flashing-Grbl-to-an-Arduino

Connecting the 5 cables to the Arduino:
--------------------------------------------------------------------
*Pixhawk/Arducopter Telemetry Port: Connect 5V and GND to the Arduinos VCC/GND pins. The TX/RX of the telemetry to the RX/TX-Pins of the Arduino (be sure to cross them!) (the ones you flashed the Hex-File previously with the TTL-Adapter)
*RX705: Connect the one white telemetry cable to Pin 5 of the Arduino. Solder this wire directly to the Arduino.

Put the Arduino in clear heat shrink tube to avoid shortcuts.

Setup / Values:
---------------
When the Arduino boots up, its green LED flashes fast, waiting for the Arducopter/Pixhawk to boot.
After that, the Arduino is in receive mode, its green LED blinks slowly.

Due to the limited Walkera Telemetry protocol (the one between Flight controller and RX705, there's another one used transferring the data via air!), the following values are transmitted:

* GPS coordinates in dd° mm' ss.sss''
* Relative GPS Altitude in m
* Speed in m/s
* Main voltage of the Pixhawk/Arducopter