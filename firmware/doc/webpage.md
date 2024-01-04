# Webpage proposal

The web page will be hosted on the ESP32, it will needs to be light weight.
It will communicate with the microcontroller using a REST/API or websocket. 

## Objectives

- Display any fatal error and give a way to fix it.
-    - Example: Mainboard defect or missing essential configurations.
- Setup the gate for the first time.
- Test the hardware easily
- Allows to dial and show the current state of the gate.
-    - Outgoing wormhole? Incoming? from whom?
     - Errors

# Pages

- Diagnostic page
     - Display errors.
     - Allows to individually test components.
     - Display troubleshoot and resolution information.
- Configuration page
     - Change user configurations.
     - Import/Export configuration into a JSON file.
- Status page
     - Dial another Stargate
     - Display errors and general status
- System/OTA page
     - Upload a new firmware
     - Upload audio assets

# Default landing page 

- Diagnostic page
  If an error preventing the system from working is detected. Any detectable hardware error or missing configuration.
- System/OTA page
  If assets are missing, like audio files.
- Status page
  If the gate is configured correctly

