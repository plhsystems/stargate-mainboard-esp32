# System Architecture

## Overview

The Stargate Mainboard ESP32 project is a multi-firmware embedded system that controls a functional Stargate replica. The system consists of multiple ESP32-based boards communicating via BLE and WiFi to control mechanical components, lighting effects, and sound.

## Firmware Components

### 1. stargate-fw (Main Controller)
**Target**: ESP32-S3 mainboard
**Purpose**: Primary gate controller and web interface host

**Key Responsibilities**:
- Gate control state machine (dialing, homing, calibration)
- Stepper motor control for ring rotation
- Servo control for chevron locking mechanism
- LED control for chevrons and wormhole effects
- Audio playback synchronization
- WiFi connectivity (Station + SoftAP modes)
- HTTP web server and REST API
- BLE client for ring communication
- Configuration management (NVS storage)

**Location**: `firmware/stargate-fw/`

### 2. ring-fw (Smart Ring Controller)
**Target**: ESP32-WROOM module on ring
**Purpose**: Independent ring LED controller

**Key Responsibilities**:
- BLE server for receiving commands from main controller
- 48 NeoPixel LED control (4 concentric rings)
- Animation effects (fade in/out, symbols, chevron animations)
- Power management (auto-sleep after inactivity)
- SGUComm protocol decoder

**Location**: `firmware/ring-fw/`

### 3. ring-factory (Factory Test Firmware)
**Target**: ESP32-WROOM module on ring
**Purpose**: Factory testing and diagnostics

**Key Responsibilities**:
- Hardware validation
- LED burn-in testing
- Factory reset capability
- Similar to ring-fw but with diagnostic features

**Location**: `firmware/ring-factory/`

### 4. Board Variants (HAL Implementations)

#### pinky-board (Production Implementation)
**Files**:
- `firmware/stargate-fw/pinky-board/main/PinkySGHW.hpp`
- `firmware/stargate-fw/pinky-board/main/PinkySGHW.cpp`

**Hardware**:
- Stepper motor (GPIO 25, 33, 26)
- Servo motor (GPIO 4, MCPWM)
- Hall sensor (GPIO 32)
- WS2812B LED strip (48 LEDs for wormhole)
- Chevron LEDs (individual control)
- Ramp LED (PWM on GPIO 23)

#### pablo-board (ESP32-S3 - In Development)
**Files**:
- `firmware/stargate-fw/pablo-board/main/PabloSGHW.hpp`
- `firmware/stargate-fw/pablo-board/main/PabloSGHW.cpp`

**Status**: Hardware implementation in progress. Firmware compiles successfully but requires:
- ESP32-S3 target configuration
- NimBLE Bluetooth stack (not Bluedroid)
- FreeRTOS trace facility enabled
- LED strip component from ESP component registry
- Larger app partition (firmware size ~1.9MB vs default 1MB partition)

**Hardware** (Similar to pinky-board with ESP32-S3 features):
- Stepper motor control
- Servo motor (MCPWM)
- Hall sensor
- WS2812B LED strips
- Chevron LEDs
- Enhanced processing power of ESP32-S3

---

## Communication Architecture

### BLE Communication (Main ↔ Ring)

```
┌─────────────────────────────┐
│   ESP32-S3 Mainboard        │
│   (stargate-fw)             │
│  ┌────────────────────────┐ │
│  │  RingBLEClient         │ │
│  │  (BLE Central/Master)  │ │
│  │                        │ │
│  │  • Scanning           │ │
│  │  • Connection Mgmt    │ │
│  │  • Write Commands     │ │
│  └────────┬───────────────┘ │
└───────────┼─────────────────┘
            │ BLE GATT
            │ Service: 12345678-1234-5678-1234-56789abcdef0
            │ Characteristic: Write
            │
┌───────────▼─────────────────┐
│   ESP32 Ring                │
│   (ring-fw)                 │
│  ┌────────────────────────┐ │
│  │  BLE Server            │ │
│  │  (BLE Peripheral)      │ │
│  │                        │ │
│  │  • Advertisement       │ │
│  │  • GATT Services       │ │
│  │  • Command Processing  │ │
│  └────────┬───────────────┘ │
│           │                 │
│  ┌────────▼───────────────┐ │
│  │  SGUComm Decoder       │ │
│  │                        │ │
│  │  Actions:              │ │
│  │  0: Heartbeat          │ │
│  │  1: Animation          │ │
│  │  2: Set Symbols        │ │
│  │  3: Power Off          │ │
│  │  4: Light Symbol       │ │
│  │  5: Factory Mode       │ │
│  └────────┬───────────────┘ │
│           │                 │
│  ┌────────▼───────────────┐ │
│  │  LED Controller        │ │
│  │  (48 NeoPixels)        │ │
│  └────────────────────────┘ │
└─────────────────────────────┘
```

**BLE Protocol Details**:
- **Connection**: Automatic scanning and reconnection
- **Service UUID**: `12345678-1234-5678-1234-56789abcdef0`
- **Characteristic UUID**: `12345678-1234-5678-1234-56789abcdef1`
- **Write Type**: Write with response
- **Heartbeat**: 1000ms interval
- **MTU**: Negotiated (typically 23-512 bytes)

### WiFi Communication

**Station Mode**:
- Connects to user's WiFi network
- Obtains IP via DHCP
- Hosts web interface on port 80
- mDNS hostname: `stargate.local`

**SoftAP Mode**:
- SSID: `Stargate-Universe`
- IP: `192.168.4.1`
- Fallback when no WiFi configured
- Allows initial setup

---

## Hardware Abstraction Layer (HAL)

### SGHW_HAL Base Class

**Location**: `firmware/stargate-fw/main-app/HW/SGHW_HAL.hpp`

**Interface**:
```cpp
class SGHW_HAL {
public:
    // Initialization
    virtual void Init() = 0;

    // Chevron Control
    virtual void SetChevronLight(EChevron chevron, bool state) = 0;

    // Ramp Light (PWM)
    virtual void SetRampLight(double brightness) = 0;

    // Wormhole LEDs
    virtual int32_t GetWHPixelCount() = 0;
    virtual CRGB* GetWHPixelArrayPtr() = 0;
    virtual void RefreshWHPixel() = 0;

    // Servo Control
    virtual void PowerUpServo() = 0;
    virtual void PowerDownServo() = 0;
    virtual void SetServo(double position) = 0;

    // Stepper Control
    virtual void PowerUpStepper() = 0;
    virtual void PowerDownStepper() = 0;
    virtual void StepStepperCW() = 0;
    virtual void StepStepperCCW() = 0;

    // Hall Sensor
    virtual bool GetHomeSensor() = 0;
};
```

**Implementations**:
- `PinkySGHW`: Full implementation for production hardware
- `PabloSGHW`: Stub for future board variant

---

## Software Architecture Layers

```
┌───────────────────────────────────────────────────┐
│              WEB INTERFACE LAYER                  │
│  • HTML/CSS/JavaScript frontend                   │
│  • Real-time status updates                       │
│  • Address book and dialing interface             │
└─────────────────┬─────────────────────────────────┘
                  │ HTTP/REST
┌─────────────────▼─────────────────────────────────┐
│              REST API LAYER                       │
│  • WebServer (port 80)                            │
│  • 30+ API endpoints                              │
│  • JSON request/response                          │
│  • OTA firmware updates                           │
└─────────────────┬─────────────────────────────────┘
                  │
┌─────────────────▼─────────────────────────────────┐
│           APPLICATION LAYER                       │
│  ┌─────────────────────────────────────────────┐  │
│  │  GateControl (Singleton State Machine)     │  │
│  │  • Command queue                            │  │
│  │  • Gate states: Idle, Dialing, Homing      │  │
│  │  • Chevron lock sequencing                  │  │
│  └─────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────┐  │
│  │  RingBLEClient (BLE Communication)         │  │
│  │  • Automatic connection management          │  │
│  │  • Command sending (heartbeat, animation)   │  │
│  └─────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────┐  │
│  │  WifiMgr (WiFi Management)                 │  │
│  │  • STA + SoftAP modes                       │  │
│  │  • Auto-reconnection                        │  │
│  └─────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────┐  │
│  │  Settings (NVS Configuration)              │  │
│  │  • WiFi credentials                         │  │
│  │  • Calibration data                         │  │
│  │  • Gate type selection                      │  │
│  └─────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────┐  │
│  │  SoundFX (Audio Playback)                  │  │
│  │  • Sound effect triggering                  │  │
│  │  • Synchronization with gate events         │  │
│  └─────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────┐  │
│  │  Wormhole (LED Effects)                    │  │
│  │  • 4-ring concentric structure              │  │
│  │  • Gradient brightness effects              │  │
│  │  • 25Hz refresh rate (40ms interval)        │  │
│  └─────────────────────────────────────────────┘  │
└─────────────────┬─────────────────────────────────┘
                  │
┌─────────────────▼─────────────────────────────────┐
│        HARDWARE ABSTRACTION LAYER                 │
│  SGHW_HAL (Abstract Interface)                    │
│  │                                                 │
│  ├─► PinkySGHW (Production)                       │
│  └─► PabloSGHW (Future)                           │
└─────────────────┬─────────────────────────────────┘
                  │
┌─────────────────▼─────────────────────────────────┐
│              DRIVER LAYER                         │
│  • Stepper (ESP Timer + GPIO)                     │
│  • Servo (MCPWM - 50Hz PWM)                       │
│  • LEDs (RMT peripheral for WS2812B)              │
│    - 48 NeoPixels for wormhole effects            │
│    - RMT with 512-symbol memory blocks            │
│    - 10MHz resolution, mutex-protected            │
│  • Hall Sensor (GPIO input with interrupts)       │
│  • Audio (I2S DAC)                                │
└─────────────────┬─────────────────────────────────┘
                  │
┌─────────────────▼─────────────────────────────────┐
│            ESP-IDF & FreeRTOS                     │
│  • Task scheduling                                │
│  • NVS (Non-Volatile Storage)                     │
│  • WiFi & Bluetooth stacks                        │
│  • HTTP server                                    │
│  • GPIO/PWM/RMT drivers                           │
└───────────────────────────────────────────────────┘
```

---

## State Machines

### GateControl State Machine

**States** (ECmd enum in [GateControl.hpp](../firmware/stargate-fw/main-app/GateControl.hpp)):
- `Stop`: Emergency stop state
- `Idle`: Waiting for commands
- `AutoHome`: Executing homing sequence
- `AutoCalibrate`: Calibrating steps per rotation
- `DialAddress`: Dialing a 7-symbol address
- `ManualWormhole`: Manual wormhole effect playback

**State Transitions**:
```
         PowerUp
            │
            ▼
    ┌───────────────┐
    │   AutoHome    │◄───── Manual trigger
    └───────┬───────┘
            │ Success
            ▼
    ┌───────────────┐
    │     Idle      │◄───── Wormhole complete
    └───┬───────────┘
        │ User command
        ├──► DialAddress ──► ManualWormhole
        ├──► AutoCalibrate
        └──► Stop (Emergency)
```

**Command Queue**:
- Thread-safe FreeRTOS queue
- Commands executed sequentially
- Priority handling for Stop command

---

## Dialing Sequence Flow

```
1. User initiates dial (web interface or API)
   │
   ▼
2. GateControl receives DialAddress command
   │
   ▼
3. For each symbol in address (7 symbols):
   │
   ├─► Calculate shortest path to symbol
   │   (handles wrap-around for circular ring)
   │
   ├─► Move stepper motor to align symbol
   │   with master chevron
   │
   ├─► Lock chevron (servo movement)
   │
   ├─► Light up chevron LED
   │
   ├─► Send animation to ring via BLE
   │   (chevron sequence effect)
   │
   └─► Play sound effect
   │
   ▼
4. All 7 chevrons locked
   │
   ▼
5. Activate wormhole effect
   │
   ├─► Wormhole LED animations
   │
   ├─► Ring LED effects (via BLE)
   │
   └─► Wormhole sound effects
   │
   ▼
6. Maintain wormhole (38 minutes or user stop)
   │
   ▼
7. Close wormhole (reverse animations)
   │
   ▼
8. Return to Idle state
```

---

## Error Handling Strategy

### Return Value Pattern

The firmware uses a consistent error handling approach based on boolean return values rather than exceptions:

**Function Signature Pattern**:
```cpp
// HAL functions return bool for success/failure
virtual bool SpinUntil(ESpinDirection spin_direction, ETransition transition,
                       uint32_t timeout_ms, int32_t* ref_tick_count);
virtual bool MoveStepperTo(int32_t ticks, uint32_t timeout_ms);
virtual bool RefreshWHPixels();
```

**Error Propagation**:
- Functions return `false` on timeout or failure
- Functions return `true` on successful completion
- Calling code checks return values and aborts operations on failure
- No exceptions are thrown during normal operation

**Example from GateControl**:
```cpp
// Dial sequence with error checking
if (!m_sghw_hal->MoveStepperTo(move_ticks, 30000)) {
    break;  // Abort dial sequence on timeout
}
```

**Timeout Values**:
- **Homing/Calibration**: 30-second timeout per operation
- **Stepper movements**: 30-second timeout per movement
- **Wormhole refresh**: Returns false on RMT peripheral errors (power instability)

**Benefits**:
- Predictable error handling without exception overhead
- Clear success/failure indication at each step
- Allows graceful degradation (continue or abort based on context)
- Better suited for embedded real-time systems

---

## Thread Safety

### Mutex Protection

Critical sections are protected with FreeRTOS mutexes:

**PinkySGHW Hardware Access**:
```cpp
// LED strip and servo operations protected by mutex
bool LockMutex() {
    return (pdTRUE == xSemaphoreTake(m_mutex_handle, pdMS_TO_TICKS(100)));
}

// All pixel operations check mutex before proceeding
if (LockMutex()) {
    led_strip_set_pixel(led_strip, index, red, green, blue);
    UnlockMutex();
}
```

**HttpClient Fan Gate List**:
```cpp
// Thread-safe access to shared fan gate data
SemaphoreHandle_t m_fanGate_mutex;
std::shared_ptr<char[]> m_fanGate;  // Protected resource

// Reader acquires mutex before accessing
if (xSemaphoreTake(m_fanGate_mutex, portMAX_DELAY) == pdTRUE) {
    result = m_fanGate;  // Safe copy
    xSemaphoreGive(m_fanGate_mutex);
}
```

**RingBLEClient Write Operations**:
- BLE write operations are mutex-protected within NimBLE stack
- Single-writer pattern ensures command serialization

---

## ESP-IDF 5.3 Compatibility

The project has been migrated to ESP-IDF 5.3 with the following changes:

### MCPWM Driver Migration
**Old API** (ESP-IDF 4.x):
```c
mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, gpio_num);
mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
```

**New API** (ESP-IDF 5.3+):
```c
mcpwm_timer_config_t timer_config = {...};
mcpwm_new_timer(&timer_config, &timer_handle);
mcpwm_new_operator(&operator_config, &oper_handle);
mcpwm_new_comparator(oper_handle, &comparator_config, &comparator);
mcpwm_new_generator(oper_handle, &generator_config, &generator);
```

### RMT (LED Strip) Configuration

**Implementation** (PinkySGHW.cpp):
```c
led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = 10 * 1000 * 1000,  // 10MHz
    .mem_block_symbols = 512,            // Increased from 128 for stability
    .flags = { .with_dma = false }
};
```

**Key Configuration**:
- **Memory Block Symbols**: 512 (increased from 128 in earlier versions)
  - Provides larger RMT buffer for WS2812B timing sequences
  - Reduces risk of underrun during LED refresh operations
  - Each symbol represents one timing period in the RMT sequence
- **Clock Resolution**: 10MHz provides precise WS2812B protocol timing
- **DMA Disabled**: Mutex-based protection used instead for thread safety
- **Error Handling**: `RefreshWHPixels()` returns `false` on `ESP_OK != ret`
  - Typically caused by power supply instability
  - Allows wormhole effect to abort gracefully on errors

### Task Priorities

All FreeRTOS tasks run at `tskIDLE_PRIORITY` for balanced execution:

```cpp
// FWConfig.hpp - Task priority definitions
#define FWCONFIG_MAINTASK_PRIORITY_DEFAULT (tskIDLE_PRIORITY)
#define FWCONFIG_WEBSERVERTASK_PRIORITY_DEFAULT (tskIDLE_PRIORITY)
#define FWCONFIG_RINGCOMM_PRIORITY_DEFAULT (tskIDLE_PRIORITY)
#define FWCONFIG_GATECONTROL_PRIORITY_DEFAULT (tskIDLE_PRIORITY)
#define FWCONFIG_HTTPCLIENT_PRIORITY_DEFAULT (tskIDLE_PRIORITY)
```

**Task Assignment**:
- **Core 0**: WebServer, Main task
- **Core 1**: GateControl, RingBLEClient, HttpClient

**Why Equal Priority**:
- Cooperative task scheduling without preemption issues
- Reduces priority inversion problems
- Simplified debugging and predictable behavior
- Tasks yield explicitly via delays and blocking calls

### NimBLE Header Changes
**Required includes** for BLE functionality:
```cpp
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// Required to avoid conflicts with C++ std::min/max
#undef min
#undef max
```

### Required Configuration Changes (sdkconfig)

#### C++ Exception Support
```
CONFIG_COMPILER_CXX_EXCEPTIONS=y
```

**CMakeLists.txt changes**:
```cmake
# In main-app/CMakeLists.txt and pinky-board/main/CMakeLists.txt
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
```

**Why**: HTTP client and other components use C++ exceptions for error handling.

#### ESP Timer ISR Dispatch
```
CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD=y
```

**Why**: Stepper motor control requires high-precision ISR-based timer callbacks for accurate step timing.

#### FreeRTOS Trace Facility
```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y
```

**Why**: Enables `vTaskList()` for debugging and system monitoring via web API.

#### FreeRTOS Tick Rate
```
CONFIG_FREERTOS_HZ=1000
```

**Why**: 1000Hz tick rate (1ms resolution) provides precise timing for:
- Stepper motor control (adaptive frequency 600-1600µs per step)
- Wormhole LED refresh (40ms intervals require accurate timing)
- BLE heartbeat intervals (1000ms)
- Task delays and timeouts throughout the system

**Note**: Previously used 100Hz (10ms resolution), upgraded to 1000Hz for improved timing accuracy.

#### Compiler Optimization
```
CONFIG_COMPILER_OPTIMIZATION_SIZE=y
```

**Why**: Size optimization (`-Os`) reduces IRAM usage and allows the firmware to fit within memory constraints.

#### Bluetooth Stack (NimBLE Required)
```
CONFIG_BT_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y
# CONFIG_BT_BLUEDROID_ENABLED is not set
CONFIG_BT_NIMBLE_MEM_ALLOC_MODE_INTERNAL=y
CONFIG_BT_NIMBLE_MAX_CONNECTIONS=3
CONFIG_BT_NIMBLE_ROLE_CENTRAL=y
CONFIG_BT_NIMBLE_GATT_CLIENT=y
```

**Why**:
- Main controller uses BLE to communicate with ring-fw
- NimBLE stack is lighter and more efficient than Bluedroid
- Must be BLE Central (Master) to initiate connection to ring
- ESP32-S3 (pablo-board) requires NimBLE, Bluedroid not compatible
- The `bt` component must be added to CMakeLists.txt REQUIRES list

**CMakeLists.txt requirement**:
```cmake
# In main-app/CMakeLists.txt
REQUIRES ... bt
```

#### Flash Size and Partitions
```
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
```

**partitions.csv**:
```
nvs,      data, nvs,     0x9000,  0x4000,
otadata,  data, ota,     0xd000,  0x2000,
phy_init, data, phy,     0xf000,  0x1000,
ota_0,    app,  ota_0,   ,        1900K,
ota_1,    app,  ota_1,   ,        1900K,
```

**Why**: Dual OTA partition scheme allows over-the-air firmware updates. 1900K per partition accommodates the ~1.7MB firmware binary with room for growth.

### ESP-IDF Version Compatibility

**Tested Versions**:
- ESP-IDF 5.3.1 ✓ (Recommended - fully tested)
- ESP-IDF 5.5.x ⚠️ (Experimental - build tested but not runtime validated)

**Migration from 4.x to 5.3+**:
The main breaking changes are in MCPWM API and component management. All firmware components have been updated for 5.3 compatibility. Testing on newer versions (5.4, 5.5) shows successful compilation but requires runtime verification.

**Component Dependencies**:
- `espressif/led_strip`: Required by pablo-board and pinky-board
  - Install: `idf.py add-dependency "espressif/led_strip"`
  - Version: 3.0.2 or compatible
  - Creates `idf_component.yml` in main component directory

---

## Memory Management

### Flash Partitions (ESP32 / ESP32-S3)
| Partition | Size | Purpose |
|-----------|------|---------|
| bootloader | ~27 KB | Second-stage bootloader |
| nvs | 16 KB | Configuration storage |
| otadata | 8 KB | OTA status tracking |
| phy_init | 4 KB | PHY calibration data |
| ota_0 | 1900 KB | OTA slot 1 (active partition) |
| ota_1 | 1900 KB | OTA slot 2 (update partition) |

**Current firmware sizes**:
- pinky-board (ESP32): ~1.8 MB (only 3% free space remaining - nearly full)
- pablo-board (ESP32-S3): ~1.9 MB (exceeds default 1MB partition, requires custom partition table)
- ring-fw (ESP32): ~560 KB (45% free space)
- ring-factory (ESP32): ~970 KB (31% free space)

### IRAM Usage
- Critical functions placed in IRAM
- Size optimization enabled (`-Os`)
- Approximately 7-10% free after compilation

---

## Configuration Storage

### NVS (Non-Volatile Storage) Keys

**WiFi Configuration**:
- `wifi.ssid`: WiFi SSID
- `wifi.password`: WiFi password

**Gate Configuration**:
- `gate.galaxy_type`: Milky Way, Pegasus, Universe
- `gate.steps_per_rot`: Calibrated steps for full rotation
- `gate.chevron_count`: Number of chevrons (default: 9)

**Ring Configuration**:
- `ring.brightness_max`: Maximum LED brightness
- `ring.auto_off_timeout`: Auto-sleep timeout (ms)

**System Configuration**:
- `system.hostname`: mDNS hostname
- `system.ota_url`: Firmware update URL

---

## Build System

### CMake Structure
```
stargate-mainboard-esp32/
├── CMakeLists.txt (root)
├── firmware/
│   ├── stargate-fw/
│   │   ├── CMakeLists.txt
│   │   ├── pinky-board/
│   │   │   └── main/
│   │   │       └── CMakeLists.txt
│   │   └── main-app/
│   │       └── CMakeLists.txt
│   ├── ring-fw/
│   │   ├── CMakeLists.txt
│   │   └── main/
│   │       └── CMakeLists.txt
│   └── ring-factory/
│       └── CMakeLists.txt
├── esp32-components/
│   └── misc-formula/
│       └── CMakeLists.txt
└── sgc-components/
    └── sgu-ringcomm/
        └── CMakeLists.txt
```

### Build Commands
```bash
# Setup ESP-IDF environment
. ~/esp/esp-idf-5.3/export.sh

# Build specific firmware
cd firmware/stargate-fw/pinky-board
idf.py build

cd firmware/ring-fw
idf.py build

cd firmware/ring-factory
idf.py build
```

---

## Shared Components

### esp32-components
**misc-formula**: Mathematical utilities
- Circle difference calculation
- LED brightness linearization
- Interpolation functions

### sgc-components
**sgu-ringcomm**: BLE communication protocol
- SGUComm message encoding/decoding
- Action definitions
- Chevron animation types

---

## Future Enhancements

1. **Pablo Board Support**: Complete HAL implementation
2. **Universe Gate**: Non-equally-spaced symbol support
3. **Subspace Network**: Fan gate interconnection via internet
4. **Advanced Wormhole Effects**: Blackhole, unstable wormhole animations
5. **Voice Control**: Integration with smart home systems
6. **Mobile App**: Dedicated iOS/Android applications

---

## Related Documentation

- [Dial Algorithm](dial-algorithm.md) - Symbol positioning math
- [Homing Algorithm](homing-algorithm.md) - Hall sensor calibration
- [Incoming Wormhole](incoming-wormhole.md) - Receiving dial sequences
- [Stargate Types](stargate-types.md) - Gate variants (SG1, Atlantis, Universe)
- [API Reference](stargate-project-api.md) - HTTP REST API endpoints
- [BLE Protocol](ble-protocol.md) - Ring communication details
