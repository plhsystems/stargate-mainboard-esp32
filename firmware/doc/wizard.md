# Wizard

The step by step wizard is meant to help user to setup their Stargate for the first time.
The first time running the mainboard, the user should be redirected to it by default.
The user is free to go through the wizard or skip it or skip some steps.

For now we only support the 'Movie' gate mode.

## Stargate type (mandatory)

| Name | Value |
|---|---|
| Stargate type | Movie, SG1, Atlantis, Universe |
| Chevron count | 7, 8 or 9 |

This step is mandatory, because it change how some input/output will react. 
The wizard will also be different based on this configuration.

## Setup Wi-Fi Station (optional) 

| Name | Value |
|---|---|
| Is active? | Is active |
| SSID | Wi-Fi SSID |
| Password | Password |

The user can setup the Wi-Fi station configuration.
Indicate the connected/disconnected status and IP address on the network.

Allows to check if a new software upgrade is available.

## Ring rotation (mandatory) 

The user must provide some parameters

| Name | Value |
|---|---|
| Home sensor type | Optical or hall |

The stepper motor will spin, the user must confirm it is spinning in the right direction.

The system will also attempts to do an homing sequence. It will detect how many steps are required to do a complete rotation. It will also ensure the speed is correct and the homing system works.

## Glyphs alignment (mandatory) 

| Name | Value |
|---|---|
| Correction offset | Correction relative to the home position |

It will ensure the glyph are aligned correctly. Each gate could be slightly mechanically different, so some manual adjustments can be required.

The ring will spin until a designed glyphs is aligned with the master chevron, then the user will have a slider to apply a correction.

## Chevron locking (mandatory) 

Every chevron will be tested one by one to ensure it move properly. If it moves the wrong way it will ask the user to revert motor polarity. 
It will also test the chevrons lights in this step.

## Audio test (optional) 

Will play a test sound to ensure the speakers are properly connected. It will be a test sound integrated within the firmware.
It will also provide a volume adjustment slider on the page.

## Test the wormhole 

| Name | Value |
|---|---|
| Wormhole type | Kristian, Tazou, PinkyMaxou |

Ask to the user what kind of wormhole is used. 

Provide a way to do some setup if required and a test button.

## Test the DHD

Confirm the DHD is connected.
Will display the DHD keypress status.
Troubleshoot informations will be given to help the user.

## Sub-space network (optional)

| Name | Value |
|---|---|
| Gate address | Reserved gate address |
| Public Key | Wireguard public key |
| VPN | VPN Address |

- Allows to configure the sub-space network, to contact other existing gates. 
- The user can setup the specific gate address and wireguard.
- Test contacting another gate
  
