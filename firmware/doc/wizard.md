# Wizard

The step by step wizard is meant to help user to setup their Stargate for the first time.
The first time running the mainboard, the user should be redirected to it by default.
The user is free to go through the wizard or skip it or skip some steps.

For now we only support the 'Movie' gate mode.

## Stargate type (mandatory)

This step is mandatory, because it change how some input/output will react. 
The wizard will also be different based on this configuration.

## Setup Wi-Fi Station (optional) 

The user can setup the Wi-Fi station configuration.
Indicate the connected/disconnected status and IP address on the network.

Allows to check if a new software upgrade is available.

## Ring rotation (mandatory) 

The stepper motor will spin, the user must confirm it is spinning in the right direction.

The system will also attempts to do an homing sequence. It will detect how many steps are required to do a complete rotation. It will also ensure the speed is correct and the homing system works correctly.

## Glyphs alignment (mandatory) 

It will ensure glyph are aligned correctly. Each gate could be slightly mechanically different, so some manual adjustments can be required.

The ring will spin until a designed glyphs is aligned with the master chevron, then the user will have a slicer to apply a correction.

## Chevron locking (mandatory) 

Every chevron will be tested one by one to ensure it move properly. If it move the wrong way it will ask the user to revert motor polarity. 
It will also test the chevrons lights in this step.

## Audio test (optional) 

Will play a test sound to ensure the speakers are properly connected. It will be a test sound integrated within the firmware.
Will also provide a volume adjustment knob on the page.

## Test the wormhole 

Ask to the user what kind of wormhole is used. 

Provide a way to do some setup if required and a test button.

## Test the DHD

Confirm the DHD is connected.
Will display the DHD keypress status.
Troubleshoot informations will be given to help the user.

## Sub-space network (optional)

- Allows to configure the sub-space network, to contact other existing gates. 
- The user can setup the specific gate address and wireguard.
- Test contacting another gate
  
