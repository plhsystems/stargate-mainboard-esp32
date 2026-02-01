# Stargate Mainboard ESP32 - Documentation

This directory contains comprehensive documentation for the Stargate Mainboard ESP32 project.

## Quick Start

New to the project? Start here:
1. [Architecture](architecture.md) - System overview and component description
2. [Build Guide](build-guide.md) - Complete build and deployment instructions
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
- ESP-IDF 5.5 compatibility details
- Memory management and partitions

#### [Build Guide](build-guide.md)
Comprehensive build and deployment guide:
- Prerequisites and environment setup
- Web assets generation (embeddedgen.py)
- Building all firmware projects
- Configuration requirements (Bluetooth, FreeRTOS)
- Common build errors and solutions
- Flash and deployment procedures
- Quick reference commands

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

1. **Install ESP-IDF 5.5**:
```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.5.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-5.5
cd esp-idf-5.5
./install.sh
```

2. **Install Dependencies**:
```bash
sudo apt install -y git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

### Build Commands

**Regenerate Web Assets** (required after web UI changes):
```bash
cd firmware/stargate-fw
python3 ../tools/embeddedgen.py -i "./main-app/webserver-assets" -o "./main-app/WebServer"
```

**Main Controller (Pinky Board - ESP32)**:
```bash
cd firmware/stargate-fw/pinky-board
. ~/esp/esp-idf-5.5/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

**Main Controller (Pablo Board - ESP32-S3)**:
```bash
cd firmware/stargate-fw/pablo-board
. ~/esp/esp-idf-5.5/export.sh
# Add LED strip dependency if not present
idf.py add-dependency "espressif/led_strip"
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

**Ring Controller (ESP32)**:
```bash
cd firmware/ring-fw
. ~/esp/esp-idf-5.5/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

**Ring Factory Test (ESP32)**:
```bash
cd firmware/ring-factory
. ~/esp/esp-idf-5.5/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

### Monitor Serial Output
```bash
idf.py -p /dev/ttyUSB0 monitor
```

### Configuration Requirements

**For Main Controllers (requires BLE for ring communication)**:
- Bluetooth: Must be enabled (`CONFIG_BT_ENABLED=y`)
- NimBLE Stack: Must be enabled (`CONFIG_BT_NIMBLE_ENABLED=y`)
- Bluedroid: Must be disabled (`# CONFIG_BT_BLUEDROID_ENABLED is not set`)
- FreeRTOS Trace: Required for task monitoring (`CONFIG_FREERTOS_USE_TRACE_FACILITY=y`)

These settings are required for:
- RingBLEClient to communicate with ring-fw
- System API task list functionality (vTaskList)

**Pablo Board Partition Warning**:
The pablo-board firmware may exceed the default app partition size. If you encounter "app partition is too small" errors, increase the factory partition size in the partition table or reduce firmware size through optimization settings.

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
- Ensure using ESP-IDF 5.5+
- Check [Architecture](architecture.md) for API migration details

**BLE header errors** (`esp_nimble_hci.h: No such file or directory`):
- Enable Bluetooth in sdkconfig: `CONFIG_BT_ENABLED=y`
- Enable NimBLE: `CONFIG_BT_NIMBLE_ENABLED=y`
- Disable Bluedroid: `# CONFIG_BT_BLUEDROID_ENABLED is not set`
- The `bt` component must be in the REQUIRES list in CMakeLists.txt

**Missing `vTaskList` symbol** (undefined reference):
- Enable FreeRTOS trace facility: `CONFIG_FREERTOS_USE_TRACE_FACILITY=y`
- Enable run time stats: `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y`

**LED strip component not found**:
- Add dependency: `idf.py add-dependency "espressif/led_strip"`
- This creates `idf_component.yml` in the main component

**App partition too small**:
- Binary exceeds partition size (common on pablo-board with full features)
- Solution 1: Increase app partition in partition table
- Solution 2: Enable size optimization: `CONFIG_COMPILER_OPTIMIZATION_SIZE=y`
- Solution 3: Disable unused features in sdkconfig

**IRAM overflow**:
- Change compiler optimization to `-Os` in sdkconfig
- Use `CONFIG_COMPILER_OPTIMIZATION_SIZE=y`

**Implicit function declaration** (`ble_advertise`):
- Add forward declaration before first use
- Example: `static void ble_advertise(void);` before the function is called

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

**Wormhole refresh errors** ("Error during refresh, may be caused by power instability"):
- Usually caused by insufficient power supply current for 48 LEDs
- Check power supply can provide enough current (48 × 60mA = ~3A at full brightness)
- Verify power connections are secure (no loose wires)
- Test with lower brightness settings
- Error handling allows graceful degradation (wormhole aborts safely)
- Recent firmware reduces refresh rate to 25Hz for better stability

**Dial sequence aborts unexpectedly**:
- Check for 30-second timeout errors in logs
- Verify stepper motor is not jammed
- Ensure hall sensor is properly aligned
- Check power supply under load

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

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/)
- [NimBLE Stack Documentation](https://mynewt.apache.org/latest/network/index.html)
- [Stargate Fan Project Community](https://buildthestargateproject.com/)
- [GitHub Repository](https://github.com/user/stargate-mainboard-esp32)

---

Last Updated: 2026-01-30
ESP-IDF Version: 5.5.1
Firmware Version: 1.0.0

## Recent Changes (2026-01)

- **FreeRTOS tick rate**: Increased from 100Hz to 1000Hz for improved timing precision
- **Wormhole refresh rate**: Reduced from 50Hz to 25Hz for power stability
- **Error handling**: Functions now return bool instead of throwing exceptions
- **Task priorities**: All tasks run at tskIDLE_PRIORITY for cooperative scheduling
- **RMT configuration**: Increased memory blocks from 128 to 512 symbols
- **BLE improvements**: Added UUID verification and connection animations
- **Thread safety**: HttpClient now uses mutex protection for fan gate list
