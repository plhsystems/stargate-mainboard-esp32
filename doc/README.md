# Stargate Mainboard ESP32 - Documentation

This directory contains comprehensive documentation for the Stargate Mainboard ESP32 project.

## Quick Start

New to the project? Start here:
1. [Architecture](architecture.md) - System overview and component description
2. [ESP-IDF 5.3 Setup](#building-firmware) - How to build the firmware
3. [API Reference](api-reference.md) - HTTP REST API documentation
4. [Web Interface](webpage.md) - Web UI features

## Documentation Index

### Core System Documentation

#### [Architecture](architecture.md)
Complete system architecture covering:
- Firmware components (stargate-fw, ring-fw, ring-factory, board variants)
- Hardware abstraction layer (HAL)
- Communication protocols (BLE, WiFi, HTTP)
- Software layers and state machines
- ESP-IDF 5.3 compatibility details
- Memory management and partitions

#### [BLE Protocol](ble-protocol.md)
Detailed BLE communication protocol between main controller and ring:
- GATT service structure
- SGUComm message format
- Action types (heartbeat, animation, symbols, power control)
- Connection management
- Error handling

#### [API Reference](api-reference.md)
Complete HTTP REST API documentation:
- System API (status, info, reboot)
- Gate control API (dial, home, calibrate, stop)
- Ring control API (animation, symbols, power)
- Settings API (configuration import/export)
- Sound effects API
- OTA update API
- WebSocket real-time updates

---

### Gate Operation

#### [Homing Algorithm](homing-algorithm.md)
Hall sensor calibration and ring positioning:
- Hysteresis handling
- Homing procedure steps
- Light sensor vs hall sensor
- Implementation details

#### [Dial Algorithm](dial-algorithm.md)
Symbol positioning mathematics:
- Distance calculation between symbols
- Minimum path finding (wrapping around ring)
- Symbol counts and spacing for each gate type
- MISCFA_CircleDiff function
- Implementation examples

#### [Incoming Wormhole](incoming-wormhole.md)
Receiving gate behavior when another gate dials:
- SG1/Movie incoming sequence (sequential chevron lighting)
- Atlantis incoming sequence (chevron + virtual spin animation)
- Universe incoming sequence (physical ring rotation, unequal spacing)
- Comparison table
- Remote triggering via Subspace Network

---

### Hardware and Configuration

#### [Stargate Types](stargate-types.md)
Supported Stargate variants:
- Movie/SG1 (39 symbols, equally spaced, physical chevron locking)
- Atlantis/Pegasus (36 symbols, LED screens, no physical ring)
- Universe (36 symbols, unequal spacing, full gate rotation)
- Visual diagrams and specifications

#### [Wormhole Types](wornhole-types.md)
Different wormhole visual effect implementations:
- Kristian's wormhole (infinite mirror tunnel)
- Kristian's wormhole #2 (retro-projection)
- PinkyMaxou's wormhole (48 NeoPixel LED screen)
- Tazou's wormhole (LEDs + mica powder + pump)

#### [Partitions](partitions.md)
Flash memory layout:
- Internal flash partitions (NVS, OTA1, OTA2)
- Micro-SD card storage (audio assets, logs)
- Size allocations

---

### User Interface

#### [Web Interface](webpage.md)
Web UI proposal and features:
- Wizard page (first-time setup)
- Diagnostic page (hardware testing)
- Configuration page (settings management)
- Control page (dialing interface)
- OTA page (firmware updates)

#### [Control Page](control-page.md)
Main user interface for gate operation:
- Address book (hardcoded addresses, fan gates)
- Manual dialer
- Commands (shutdown, homing)
- Status display

#### [Configuration Page](configuration-page.md)
Settings management:
- JSON file viewer/editor
- Import/export functionality

#### [Wizard Page](wizard-page.md)
Step-by-step initial setup

#### [Diagnostic Page](diagnostic-page.md)
Hardware testing and troubleshooting

#### [OTA Page](ota-page.md)
Firmware and asset updates

---

### Miscellaneous

#### [Misc](misc.md)
Various technical notes:
- LED linearity and human perception
- PWM duty cycle formula for linear fade effects

#### [Stargate Project API](stargate-project-api.md)
External API integration:
- Fan Gates Network API
- Address book downloads
- Online gate status

#### [Copyright](copyright.md)
Legal and licensing information

---

## Building Firmware

### Prerequisites

1. **Install ESP-IDF 5.3**:
```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.3.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-5.3
cd esp-idf-5.3
./install.sh
```

2. **Install Dependencies**:
```bash
sudo apt install -y git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

### Build Commands

**Main Controller (Pinky Board)**:
```bash
cd firmware/stargate-fw/pinky-board
. ~/esp/esp-idf-5.3/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

**Ring Controller**:
```bash
cd firmware/ring-fw
. ~/esp/esp-idf-5.3/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

**Ring Factory Test**:
```bash
cd firmware/ring-factory
. ~/esp/esp-idf-5.3/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

### Monitor Serial Output
```bash
idf.py -p /dev/ttyUSB0 monitor
```

---

## Hardware Setup

### Pinky Board Connections

**Stepper Motor**:
- Direction: GPIO 33
- Step: GPIO 25
- Sleep: GPIO 26

**Servo Motor**:
- PWM: GPIO 4 (MCPWM)

**Hall Sensor**:
- Input: GPIO 32

**LEDs**:
- Wormhole NeoPixels: GPIO 27 (48 LEDs)
- Ramp LED: GPIO 23 (PWM)
- Sanity LED: GPIO 5

### Ring Board Connections

**NeoPixel Ring**:
- Data: GPIO 13
- Count: 48 LEDs (4 concentric rings)

---

## Development

### Project Structure
```
stargate-mainboard-esp32/
├── firmware/
│   ├── stargate-fw/        # Main controller firmware
│   │   ├── main-app/       # Application code
│   │   └── pinky-board/    # Hardware-specific code
│   ├── ring-fw/            # Ring controller firmware
│   └── ring-factory/       # Ring factory test firmware
├── esp32-components/       # Shared ESP32 components
│   └── misc-formula/       # Mathematical utilities
├── sgc-components/         # Stargate-specific components
│   └── sgu-ringcomm/       # BLE protocol implementation
└── doc/                    # This documentation
```

### Code Style
- C++ standard: C++17
- Naming: PascalCase for classes, camelCase for functions
- Indentation: 4 spaces (no tabs)
- Comments: Doxygen-style for public APIs

### Adding New Features

1. **Plan**: Document the feature in this folder
2. **Implement**: Follow HAL pattern for hardware features
3. **Test**: Use diagnostic page for hardware testing
4. **API**: Add HTTP endpoints if user-facing
5. **Document**: Update relevant .md files

---

## Troubleshooting

### Build Errors

**MCPWM API errors**:
- Ensure using ESP-IDF 5.3+
- Check [Architecture](architecture.md) for API migration details

**BLE header errors**:
- Add `#undef min` and `#undef max` after NimBLE includes
- Include all required headers (see [BLE Protocol](ble-protocol.md))

**IRAM overflow**:
- Change compiler optimization to `-Os` in sdkconfig
- Use `CONFIG_COMPILER_OPTIMIZATION_SIZE=y`

### Runtime Errors

**Ring won't connect**:
- Check BLE is enabled on both devices
- Verify ring is powered and within range
- Check logs: `ESP_LOGI` messages from "RingBLEClient"

**Homing fails**:
- Verify hall sensor wiring (GPIO 32)
- Check sensor polarity
- Run manual homing via web interface

**Stepper doesn't move**:
- Check sleep pin (GPIO 26) is high
- Verify stepper driver power supply
- Test with manual step command

---

## Contributing

When adding documentation:
1. Keep technical accuracy as top priority
2. Include code examples where relevant
3. Link to related documentation
4. Add to this README index
5. Use markdown formatting consistently

---

## License

See [Copyright](copyright.md) for licensing information.

---

## External Resources

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/)
- [NimBLE Stack Documentation](https://mynewt.apache.org/latest/network/index.html)
- [Stargate Fan Project Community](https://buildthestargateproject.com/)
- [GitHub Repository](https://github.com/user/stargate-mainboard-esp32)

---

Last Updated: 2026-01-22
ESP-IDF Version: 5.3.1
Firmware Version: 1.0.0
