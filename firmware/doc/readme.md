# Mainboard Firmware

The project goal is to make a fully functional Stargate (designed by Kristian) by using an ESP32 instead of the raspberry pi.

## Why? 

The ESP32 is cheap when compared to a raspberry pi and more accessible. Everything the raspberry pi does to control the Stargate can be done with cheaper hardware.
Also the mainboard will integrate all the necessary drivers, IC, regulator, etc. in a single PCB.
The current iteration of the Stargate using the raspberry pi needs a lot of external breakout to works. It increase the cost and make the electronic more difficult to master for people who are less skilled with electronic.

# Objectives

These are all functions we would like to support. 

- Support of the existing DHD board (based on STM32) 
- Control 9 chevrons lightning (optionally just 7)
- Control 9 all chevrons up/down movement
   - Could support only 8, 7 or 1 moving chevron, can be configured by the user. 
- Support spinning the ring to dial an address
- Integrated audio amplifier for sound effects
- Support for SDCard, to increase the available storage space.
- Support the wormhole effect, may support more than one effect or wormhole type.
- Support easy OTA
- Connection to other existing Stargate on the network
- Wi-Fi support (Station + SoftAP)
- Support three type of Stargate (Movie/SG1/Atlantis/Universe)
