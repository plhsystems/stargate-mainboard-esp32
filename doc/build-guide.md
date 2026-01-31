# Build and Deployment Guide

Complete guide for building, deploying, and troubleshooting the Stargate Mainboard ESP32 firmware.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Build Process](#build-process)
- [Web Assets](#web-assets)
- [Firmware Projects](#firmware-projects)
- [Configuration](#configuration)
- [Migration from ESP-IDF 5.x](#migration-from-esp-idf-5x)
- [Common Issues](#common-issues)

---

## Prerequisites

### Required Software

1. **ESP-IDF 6.1** (Current)
```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v6.1 --recursive https://github.com/espressif/esp-idf.git esp-idf
cd esp-idf
./install.sh
```

2. **System Dependencies**
```bash
sudo apt install -y git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

3. **Python Tools**
- Python 3.8+ (for embeddedgen.py)
- ESP-IDF Python environment (installed by install.sh)

### Directory Structure
```
stargate-mainboard-esp32/
├── firmware/
│   ├── stargate-fw/
│   │   ├── main-app/              # Main application code
│   │   │   ├── webserver-assets/  # Web UI source files
│   │   │   └── WebServer/         # Generated embedded files
│   │   ├── pablo-board/           # ESP32-S3 variant
│   │   └── pinky-board/           # ESP32 variant (production)
│   ├── ring-fw/                   # Ring controller
│   ├── ring-factory/              # Ring factory test
│   └── tools/
│       └── embeddedgen.py         # Web asset embedding tool
├── esp32-components/
└── doc/
```

---

## Build Process

### Step 1: Environment Setup

Source the ESP-IDF environment in each terminal session:
```bash
. ~/esp/esp-idf/export.sh
```

To make this permanent, add to `~/.bashrc`:
```bash
alias get_idf='. ~/esp/esp-idf/export.sh'
```

### Step 2: Web Assets Generation

**When to regenerate**:
- After modifying HTML, CSS, or JavaScript
- After updating glyph images or SVG assets
- Before final firmware build

**Command**:
```bash
cd firmware/stargate-fw
python3 ../tools/embeddedgen.py -i "./main-app/webserver-assets" -o "./main-app/WebServer"
```

**Generated files**:
- `WebServer/EmbeddedFiles.c` - C arrays with file contents
- `WebServer/EmbeddedFiles.h` - Header declarations
- `WebServer/EmbeddedFiles.bin` - Binary concatenation
- `WebServer/EmbeddedFiles.txt` - File listing

### Step 3: Build Firmware

#### Pinky Board (ESP32 - Production)
```bash
cd firmware/stargate-fw/pinky-board
. ~/esp/esp-idf/export.sh
idf.py build
```

**Build output**: `build/stargate-fw.bin` (~1.8 MB)

#### Pablo Board (ESP32-S3 - Development)
```bash
cd firmware/stargate-fw/pablo-board
. ~/esp/esp-idf/export.sh

# First time only: add LED strip dependency
idf.py add-dependency "espressif/led_strip"

idf.py build
```

**Build output**: `build/pablo-board.bin` (~1.9 MB)

**Warning**: May require partition table adjustment due to size.

#### Ring Controller
```bash
cd firmware/ring-fw
. ~/esp/esp-idf/export.sh
idf.py build
```

**Build output**: `build/ring-fw.bin` (~560 KB)

#### Ring Factory Test
```bash
cd firmware/ring-factory
. ~/esp/esp-idf/export.sh
idf.py build
```

**Build output**: `build/ring_factory.bin` (~970 KB)

### Step 4: Flash Firmware

**Connect device**: USB to serial (typically `/dev/ttyUSB0`)

**Flash command**:
```bash
idf.py -p /dev/ttyUSB0 flash
```

**Monitor output**:
```bash
idf.py -p /dev/ttyUSB0 monitor
```

**Flash and monitor**:
```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

**Exit monitor**: `Ctrl+]`

---

## Configuration

### Required sdkconfig Settings

All main controller boards require these settings:

#### Bluetooth (NimBLE)
```
CONFIG_BT_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y
# CONFIG_BT_BLUEDROID_ENABLED is not set
CONFIG_BT_NIMBLE_MEM_ALLOC_MODE_INTERNAL=y
CONFIG_BT_NIMBLE_MAX_CONNECTIONS=3
CONFIG_BT_NIMBLE_ROLE_CENTRAL=y
CONFIG_BT_NIMBLE_GATT_CLIENT=y
```

#### FreeRTOS Trace
```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y
```

#### C++ Exceptions
```
CONFIG_COMPILER_CXX_EXCEPTIONS=y
```

#### Size Optimization (recommended)
```
CONFIG_COMPILER_OPTIMIZATION_SIZE=y
```

### Interactive Configuration

```bash
idf.py menuconfig
```

Navigate to:
- `Component config → Bluetooth → NimBLE`
- `Component config → FreeRTOS → Kernel`
- `Compiler options → Optimization Level`

---

## Migration from ESP-IDF 5.x

If you're upgrading from ESP-IDF 5.3.x to 6.1, the following changes have been made to the codebase:

### Component Changes

#### cJSON Component
The `json` component has been replaced with `cjson` in ESP-IDF 6.1.

**Changes made**:
- Updated `esp32-components/nvsjson/CMakeLists.txt`: `json` → `cjson`
- Updated all firmware projects' `main/CMakeLists.txt`: `json` → `cjson`
- Added `espressif/cjson` dependency via `idf.py add-dependency "espressif/cjson"`

### Driver API Changes

#### Legacy RMT Driver
The legacy `driver/rmt.h` has been removed and replaced with new driver APIs.

**Changes made**:
- Removed `#include "driver/rmt.h"` from `gpio.c` files (not needed as led_strip handles RMT internally)
- Updated led_strip component dependencies to include `esp_driver_rmt` and `esp_driver_spi`

#### Driver Component Split
ESP-IDF 6.1 has split driver components into separate packages.

**Required CMakeLists.txt updates**:
```cmake
# Old (ESP-IDF 5.x)
REQUIRES driver

# New (ESP-IDF 6.1)
REQUIRES driver esp_driver_gpio esp_driver_rmt esp_driver_uart esp_driver_ledc
```

**Changes made across projects**:
- pinky-board: Added `esp_driver_mcpwm`, `esp_driver_ledc`, `esp_driver_gpio`, `esp_driver_gptimer`
- pablo-board: Added `esp_driver_ledc`, `esp_driver_gpio`
- ring-fw: Added `esp_driver_gpio`, `esp_driver_rmt`
- ring-factory: Added `esp_driver_gpio`
- main-app: Added `esp_driver_uart`

### Include Header Changes

#### FreeRTOS Include Order
ESP-IDF 6.1 strictly requires `FreeRTOS.h` to be included before other FreeRTOS headers.

**Fix applied**:
```cpp
// Correct order (ESP-IDF 6.1)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Wrong order (causes compilation error)
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
```

**Files fixed**:
- `pinky-board/main/PinkySGHW.hpp`
- `pablo-board/main/PabloSGHW.hpp`

#### LED Strip Component
The led_strip component requires additional headers in ESP-IDF 6.1.

**Fix applied** to `managed_components/espressif__led_strip/src/led_strip_spi_dev.c`:
```c
#include "esp_heap_caps.h"  // Added for MALLOC_CAP_* macros
```

### Configuration Changes

#### Partition Table
Pablo-board requires 4MB flash configuration instead of 2MB.

**Changes made** to `pablo-board/sdkconfig`:
```
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
```

#### FreeRTOS Trace Facility
All projects now explicitly enable FreeRTOS trace facility.

**Added to all sdkconfig files**:
```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y
```

### Code Compatibility

#### cJSON API Changes
The `cJSON_PrintUnformatted` function now returns `const char*` instead of `char*`.

**Fix applied** to `ring-factory/main/webserver.c`:
```c
// Old code
const char* p_str = cJSON_PrintUnformatted(p_root);
return p_str;  // Error: discards const qualifier

// Fixed code
char* p_str = (char*)cJSON_PrintUnformatted(p_root);
return p_str;  // Cast away const for compatibility
```

### Migration Checklist

When migrating your own fork or branch:

- [ ] Update ESP-IDF to version 6.1
- [ ] Initialize/update git submodules: `git submodule update --init --recursive`
- [ ] Replace `json` with `cjson` in all CMakeLists.txt files
- [ ] Add `espressif/cjson` dependency to all projects
- [ ] Update led_strip component CMakeLists.txt with driver dependencies
- [ ] Add `esp_heap_caps.h` to led_strip_spi_dev.c
- [ ] Fix FreeRTOS include order (FreeRTOS.h before semphr.h)
- [ ] Remove legacy `driver/rmt.h` includes from gpio.c files
- [ ] Add new driver components (esp_driver_*) to CMakeLists.txt
- [ ] Update sdkconfig files with FreeRTOS trace facility settings
- [ ] For pablo-board: Update flash size to 4MB and enable custom partition table
- [ ] Clean build all projects: `idf.py fullclean && idf.py build`
- [ ] Test all firmware builds complete successfully

---

## Common Issues

### Build Errors

#### Missing `esp_nimble_hci.h`
**Error**:
```
fatal error: esp_nimble_hci.h: No such file or directory
```

**Solution**:
1. Enable Bluetooth in sdkconfig: `CONFIG_BT_ENABLED=y`
2. Enable NimBLE: `CONFIG_BT_NIMBLE_ENABLED=y`
3. Disable Bluedroid: `# CONFIG_BT_BLUEDROID_ENABLED is not set`
4. Add `bt` to CMakeLists.txt REQUIRES:
```cmake
REQUIRES ... bt
```

#### Undefined Reference to `vTaskList`
**Error**:
```
undefined reference to `vTaskList'
```

**Solution**:
Enable FreeRTOS trace facility in sdkconfig:
```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y
```

#### LED Strip Component Not Found
**Error**:
```
Failed to resolve component 'led_strip'
```

**Solution**:
```bash
cd firmware/stargate-fw/pablo-board  # or pinky-board
idf.py add-dependency "espressif/led_strip"
```

This creates `main/idf_component.yml`.

#### App Partition Too Small
**Error**:
```
Error: app partition is too small for binary pablo-board.bin size 0x1d5d00:
  - Part 'factory' 0/0 @ 0x10000 size 0x100000 (overflow 0xd5d00)
```

**Solutions**:

1. **Increase partition size** (Recommended):
Edit `partitions.csv`:
```
ota_0,    app,  ota_0,   ,        2048K,
ota_1,    app,  ota_1,   ,        2048K,
```

2. **Enable size optimization**:
```
CONFIG_COMPILER_OPTIMIZATION_SIZE=y
```

3. **Disable unused features** via menuconfig:
- Disable Bluetooth if not needed
- Reduce log verbosity
- Disable unused drivers

#### Implicit Function Declaration
**Error**:
```
implicit declaration of function 'ble_advertise'
```

**Solution**:
Add forward declaration before first use:
```c
// Forward declarations
static void ble_advertise(void);
```

### Flash Errors

#### Permission Denied
**Error**:
```
serial.serialutil.SerialException: [Errno 13] could not open port /dev/ttyUSB0
```

**Solution**:
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in
```

#### Device Not Found
**Error**:
```
A fatal error occurred: Could not open /dev/ttyUSB0
```

**Solution**:
1. Check connection: `ls /dev/ttyUSB*`
2. Try different port: `-p /dev/ttyUSB1`
3. Check USB cable (some are power-only)
4. Install USB drivers (CH340, CP210x, FTDI)

---

## Advanced Topics

### Parallel Builds

Build multiple projects concurrently:
```bash
# Terminal 1
cd firmware/stargate-fw/pinky-board && idf.py build

# Terminal 2
cd firmware/ring-fw && idf.py build

# Terminal 3
cd firmware/ring-factory && idf.py build
```

### Clean Builds

**Full clean** (removes all build artifacts):
```bash
idf.py fullclean
```

**Standard clean** (keeps configuration):
```bash
idf.py clean
```

### Build Size Analysis

```bash
idf.py size
idf.py size-components
idf.py size-files
```

### Partition Table Viewer

```bash
idf.py partition-table
```

---

## Deployment Checklist

Before deploying to production hardware:

- [ ] Web assets regenerated with embeddedgen.py
- [ ] All firmware builds complete without errors
- [ ] Serial monitor tested (no runtime errors)
- [ ] WiFi connection tested
- [ ] Web interface accessible
- [ ] BLE connection to ring tested
- [ ] Stepper motor movement verified
- [ ] Servo chevron lock tested
- [ ] LED effects working
- [ ] Audio playback functional (if applicable)
- [ ] OTA update tested (if implemented)

---

## Quick Reference

### One-Line Build Commands

**Pinky Board**:
```bash
cd ~/stargate-mainboard-esp32/firmware/stargate-fw/pinky-board && . ~/esp/esp-idf/export.sh && idf.py build
```

**Pablo Board**:
```bash
cd ~/stargate-mainboard-esp32/firmware/stargate-fw/pablo-board && . ~/esp/esp-idf/export.sh && idf.py build
```

**Ring FW**:
```bash
cd ~/stargate-mainboard-esp32/firmware/ring-fw && . ~/esp/esp-idf/export.sh && idf.py build
```

**All Projects** (sequential):
```bash
cd ~/stargate-mainboard-esp32/firmware && \
  . ~/esp/esp-idf/export.sh && \
  (cd stargate-fw && python3 ../tools/embeddedgen.py -i "./main-app/webserver-assets" -o "./main-app/WebServer") && \
  (cd stargate-fw/pinky-board && idf.py build) && \
  (cd ring-fw && idf.py build) && \
  (cd ring-factory && idf.py build)
```

---

## Support

For issues and questions:
- Check [Troubleshooting](README.md#troubleshooting) section
- Review [Architecture](architecture.md) documentation
- Check ESP-IDF documentation: https://docs.espressif.com/projects/esp-idf/en/v6.1/
- Report bugs: https://github.com/user/stargate-mainboard-esp32/issues

---

Last Updated: 2026-01-31
ESP-IDF Version: 6.1
