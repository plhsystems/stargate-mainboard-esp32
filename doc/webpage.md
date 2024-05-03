# Webpage proposal

The web page will be hosted on the ESP32, it will needs to be light weight.
It will communicate with the microcontroller using a REST/API or websocket. 

## Objectives

- Display any fatal error and give a way to fix it.
   - Example: Mainboard defect or missing essential configurations.
- Setup the gate for the first time.
- Test the hardware easily
- Allows to dial and show the current state of the gate.
      - Outgoing wormhole? Incoming? from whom?
      - Errors

# Pages

- Wizard page
  A step by step test and configuration wizard. Every components are tested one by one, user will need to provide inputs to confirm everything works. Example, ensure the stepper spin in the right direction at the right speed. Test all chevron lights, wormhole etc.
- Wizard
     - Most configurations that needs to be done once to make the Stargate operational.
- Diagnostic page
     - Display errors.
     - Allows to individually test components.
     - Display troubleshoot and resolution information.
- Configuration page
     - Change user configurations.
     - Import/Export configuration into a JSON file.
- Control page
     - Dial
     - Display errors and general status
- OTA page
     - Upload a new firmware
     - Upload audio assets

# Default landing page 

- Wizard page
  Default page, until the wizard is successfully completed.
- Diagnostic page
  If an error preventing the system from working is detected. Any detectable hardware error or missing configuration.
- OTA page
  If assets are missing, like the audio files.
- Control page
  If the gate is configured correctly

