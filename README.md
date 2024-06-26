# Mainboard Firmware

The project goal is to make a fully functional Stargate (designed by Kristian) by using an ESP32 instead of the raspberry pi.

## Why?

The ESP32 is cheap compared to a raspberry pi and easier to get (there are recurrent shortage).
Everything the raspberry pi does to control the Stargate can be done with cheaper hardware.
Also the mainboard will integrate all the necessary drivers, IC, regulator, etc. in a single PCB.
The current iteration of the Stargate using the raspberry pi needs a lot of external breakout to works. It increase the cost and make the electronic more difficult to master especially for people whom are less skilled with electronic.

# Objectives

These are all functions we would like to support.

- Control and configuration by using a webpage hosted on the ESP32.
- Support of the existing DHD board (based on STM32)
- Control 9 chevrons lightning (optionally just 7)
- Control 9 all chevrons up/down movement
   - Could support only 8, 7 or 1 moving chevron, can be configured by the user.
- Support ramp lightning (could be used for other leds if no ramp lighting is used)
- Support spinning the ring to dial an address
- Integrated audio amplifier for sound effects
- Support for SD Card, to increase the available storage space.
- Support the wormhole effect, may support more than one effect or wormhole type.
- Support OTA (not necessarily fully automatic)
- Connection to other existing Stargate on the network
- Wi-Fi support (Station + SoftAP)
- Support all types of Stargate (Movie/SG1/Atlantis/Universe)
- Support automatic ring homing
   - Support for HALL sensor
   - Support for optical sensor
   - Should be able to calibrate itself automatically (tick count per rotation, speed etc. )
- Should be easy to configure
   - Easily import or export a configuration (json file)

# Wish objectives

These one are secondary, at least not mandatory but could be fun to have.

- Support automatic OTA check and download. Including all assets (audio/web/etc..)
- Have a failsafe mode? (factory reset)
- Nice looking page to dial or get the current gate status
- REST API (HTTP/MQTT?)

# Debug & diagnostic mode

- Should have the ability to do hardware diagnostic, as much as the software allows it. Example : ensure all i2c devices respond.
- Should have a mode to test all sensors, motors, light etc.. without restriction for debugging purpose.
- Debug console, accessible from TCP/IP or UART.
