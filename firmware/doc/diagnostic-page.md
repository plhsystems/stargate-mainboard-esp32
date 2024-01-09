# Diagnostic page

This page should display any fatal/error/warning preventing the Stargate from working correctly.
As long as it could spin it is considered functional.

This page may be replaced by an overlay on top of the page. Not yet decided.

## Level

| Level | Description |
|---|---|
| Fatal | The mainboard doesn't work |
| Error | Some functionality doesn't work and require user attention |
| Warning | Works but not fully |
| OK | System is working |
| Disabled | Doesn't apply (functionality is deactivated) |

## Functionality

| Function | Level | Description |
|---|---|---|
| I2C PWM IC #X | Fatal | I2C chip aren't communicating |
| Stepper not calibrated | Error | The ring cannot move |
| Homing failed | Error | The ring homing process failed |
| Missing basic configuration | Error | Minimum configuration requirement is not met |
| Missing audio assets | Warning | No audio assets, sound effect won't work |
| Missing SD Card | Warning | Some function won't work without an SD Card |
| Cannot get IP Address | Warning | Cannot access to the Internet |
| Wi-Fi station | Warning | Cannot connect to the Wi-Fi AP |
| Subspace network | Warning | Cannot connect to the subspace network Wireguard VPN |
| Wizard not completed | Warning | The configuration wizard process is not finished |

# System informations

Display any useful system information
- IP address
- MAC Addresses
- RAM usage
- Firmware SHA256
- SD CARD present
- File system usage


