# Stargate REST API Reference

## Overview

The Stargate mainboard exposes an HTTP REST API on port 80 for control and monitoring. Most endpoints accept and return JSON data.

**Base URL**: `http://stargate.local/` or `http://<IP_ADDRESS>/`

**Implementation**: `firmware/stargate-fw/main-app/WebServer/SystemAPI.cpp` and `APIURL.hpp`

---

## Authentication

Currently, the API does not require authentication. All endpoints are publicly accessible on the local network.

---

## System API

### GET /api/getstatus
Get current system status and gate state.

**Implementation**: Real-time status polling endpoint (typically called every 500ms by web interface)

**Response**:
```json
{
    "status_text": "Idle",
    "is_cancel_requested": false,
    "error_text": "",
    "is_ring_connected": true,
    "time_hours": 14,
    "time_minutes": 30,
    "last_error": false
}
```

**Fields**:
- `status_text`: Current gate state description ("Idle", "Dialing", "Homing", etc.)
- `is_cancel_requested`: Whether abort has been requested
- `error_text`: Last error message (empty if no error)
- `is_ring_connected`: BLE connection status to ring
- `time_hours`, `time_minutes`: Current system time
- `last_error`: Error flag

---

### GET /api/getsysinfo
Get system information including firmware version and hardware details.

**Response**: JSON object with key-value pairs
```json
{
    "firmware_version": "1.0.0",
    "hardware_revision": "pinky-board",
    "idf_version": "v5.5.1",
    "chip_model": "ESP32",
    "free_heap_bytes": 234567,
    "min_free_heap_bytes": 220000,
    "uptime_seconds": 12345,
    "...": "additional system info"
}
```

---

### GET /api/getfreertosdbginfo
Get FreeRTOS task list and debug information.

**Response**: Text format (not JSON) with task statistics

**Usage**: Debugging and performance monitoring. Shows all running tasks with stack usage and CPU time.

---

## Gate Control API

### POST /api/control/autohome
Execute automatic homing sequence using hall sensor.

**Request**: Empty body

**Response**:
```json
{
    "success": true,
    "message": "Homing complete"
}
```

**Behavior**:
- Finds hall sensor rising edge transition
- Centers ring in sensor deadband
- 30-second timeout per spin operation
- Returns error on timeout or sensor failure
- See [homing-algorithm.md](homing-algorithm.md) for details

---

### POST /api/control/autocalibrate
Perform full rotation calibration to measure steps per rotation and home sensor gap.

**Request**: Empty body

**Response**:
```json
{
    "success": true,
    "steps_per_rotation": 7680,
    "home_gap_range": 42,
    "message": "Calibration complete"
}
```

**Behavior**:
- Performs multiple sensor transitions (Rising → Rising, Failing → Rising)
- Stores calibration data in NVS (`StepsPerRotation`, `RingHomeGapRange`)
- 30-second timeout per operation
- Required before first use

---

### POST /api/control/dialaddress
Dial a Stargate address (typically 7-9 symbols).

**Request**:
```json
{
    "symbols": [12, 37, 24, 1, 32, 10, 1],
    "wormhole_type": 0
}
```

**Parameters**:
- `symbols`: Array of symbol numbers
  - SG1/Milkyway: 1-39 (1-indexed)
  - Pegasus/Universe: 0-35 (varies by implementation)
- `wormhole_type`: Wormhole effect type
  - `0`: NormalSGU (Universe gate visual)
  - `1`: NormalSG1 (SG1 gate visual)

**Response**:
```json
{
    "success": true,
    "message": "Dialing sequence started"
}
```

**Behavior**:
- Moves ring to each symbol position sequentially
- Locks chevron via servo after each symbol
- Sends BLE commands to ring for symbol lighting
- Opens wormhole effect on completion
- Maintains wormhole for 5 minutes or until abort
- Each stepper movement has 30-second timeout
- Aborts entire sequence if any movement fails
- See [dial-algorithm.md](dial-algorithm.md) for details

---

### POST /api/control/manualwormhole
Activate wormhole effect without dialing.

**Request**:
```json
{
    "wormhole_type": 0
}
```

**Parameters**:
- `wormhole_type`: 0 = NormalSGU, 1 = NormalSG1

**Response**:
```json
{
    "success": true
}
```

**Behavior**:
- Starts wormhole LED animation (25Hz refresh rate)
- Runs indefinitely until abort
- No chevron locking or ring movement

---

### POST /api/control/abort
Abort current operation (dialing, homing, wormhole, calibration).

**Request**: Empty body

**Response**:
```json
{
    "success": true,
    "message": "Operation aborted"
}
```

**Behavior**:
- Sets cancel flag (`m_is_cancel_action`)
- Current operation checks flag and terminates gracefully
- Returns gate to idle state
- Safe to call at any time

---

### POST /api/control/keypress
Simulate keypress for DHD (Dial Home Device) integration.

**Request**:
```json
{
    "key": "1"
}
```

**Parameters**:
- `key`: Key identifier (symbol number or control key)

**Response**:
```json
{
    "success": true
}
```

**Note**: Used for DHD hardware integration

---

### POST /api/control/testramplight
Test ramp LED brightness (hardware validation endpoint).

**Request**:
```json
{
    "value": 0.5
}
```

**Parameters**:
- `value`: Brightness level (0.0 to 1.0)

**Response**:
```json
{
    "success": true
}
```

**GPIO**: GPIO 23 (PWM via LEDC)

---

### POST /api/control/testservo
Test chevron servo position (hardware validation endpoint).

**Request**:
```json
{
    "value": 0.75
}
```

**Parameters**:
- `value`: Servo position (0.0 = released, 1.0 = locked)

**Response**:
```json
{
    "success": true
}
```

**Note**: Test endpoint for hardware validation. Maps to 1000-2000µs PWM pulse width.

---

## Ring Control API

### POST /api/ringcontrol/poweroff
Power off the ring LED controller via BLE.

**Request**: Empty body

**Response**:
```json
{
    "success": true,
    "message": "Ring powering off"
}
```

**Behavior**:
- Sends BLE Action 3 (Power Off) to ring
- Ring turns off all LEDs and enters deep sleep
- BLE connection will be lost
- Ring wakes on reset or GPIO trigger

---

### POST /api/ringcontrol/testanimate
Trigger test animation on ring via BLE.

**Request**:
```json
{
    "animation_id": 0
}
```

**Parameters**:
- `animation_id`: Animation type from EChevronAnimation enum (see [ble-protocol.md](ble-protocol.md))
  - `0`: Chevron_FadeIn
  - `1`: Chevron_FadeOut
  - `2`: Chevron_ErrorToWhite
  - `3`: Chevron_ErrorToOff
  - `4`: Chevron_AllSymbolsOn
  - `5`: Chevron_PoweringOff
  - `6`: Chevron_NoSymbols

**Response**:
```json
{
    "success": true
}
```

---

### POST /api/ringcontrol/gotofactory
Send ring to factory mode (resets configuration).

**Request**: Empty body

**Response**:
```json
{
    "success": true,
    "message": "Ring entering factory mode"
}
```

**Behavior**:
- Sends BLE Action 5 (Factory Mode)
- Ring erases NVS configuration
- Resets to factory defaults

---

## Sound API

### GET /api/sound/list
Get list of available sound effects from SD card.

**Response**:
```json
{
    "sounds": [
        {"id": 0, "name": "chevron_lock.wav"},
        {"id": 1, "name": "wormhole_open.wav"},
        {"id": 2, "name": "wormhole_close.wav"}
    ]
}
```

**Note**: Scans `/sounds/` directory on SD card

---

### POST /api/sound/play
Play a sound effect by ID.

**Request**:
```json
{
    "id": 0
}
```

**Parameters**:
- `id`: Sound effect ID from list

**Response**:
```json
{
    "success": true
}
```

---

### POST /api/sound/stop
Stop currently playing sound.

**Request**: Empty body

**Response**:
```json
{
    "success": true
}
```

---

## Settings API

### GET /api/settingsjson
Export all settings as JSON.

**Response**: JSON array of settings
```json
{
    "settings": [
        {
            "key": "wifi_ssid",
            "description": "WiFi Network SSID",
            "type": "string",
            "min": null,
            "max": null,
            "default": "",
            "value": "MyNetwork"
        },
        {
            "key": "steps_per_rotation",
            "description": "Stepper motor steps per full rotation",
            "type": "int32",
            "min": 1000,
            "max": 20000,
            "default": 7680,
            "value": 7680
        }
    ]
}
```

**Fields**:
- `key`: Setting identifier (NVS key)
- `description`: Human-readable description
- `type`: Data type (string, int32, float, bool)
- `min`, `max`: Valid range (null if not applicable)
- `default`: Factory default value
- `value`: Current value from NVS

**Note**: This is a **read-only** export in the current implementation

---

### POST /api/settingsjson
Import settings from JSON (validation only in current implementation).

**Request**: Same format as GET response

**Response**:
```json
{
    "success": true,
    "imported_count": 15,
    "message": "Settings imported successfully"
}
```

**Note**: Full import/export functionality is planned but not fully implemented. Currently validates but may not apply all settings.

---

## Galaxy API

### GET /api/galaxy/getinfo/milkyway
Get Milkyway (SG1) gate information and addresses.

**Response**:
```json
{
    "galaxy_name": "Milky Way",
    "galaxy_type": "SG1",
    "symbol_count": 39,
    "chevron_count": 7,
    "addresses": [
        {
            "name": "Earth (Point of Origin)",
            "symbols": [1]
        },
        {
            "name": "Abydos",
            "symbols": [26, 6, 14, 31, 11, 29, 1]
        }
    ]
}
```

**Implementation**: Returns hardcoded addresses from `MilkyWayGate.hpp` (35 pre-programmed addresses)

---

### GET /api/galaxy/getinfo/pegasus
Get Pegasus (Atlantis) gate information and addresses.

**Response**: Similar to milkyway
```json
{
    "galaxy_name": "Pegasus",
    "galaxy_type": "Atlantis",
    "symbol_count": 36,
    "chevron_count": 6,
    "addresses": [...]
}
```

**Implementation**: Returns 11 pre-programmed addresses from `PegasusGate.hpp`

---

### GET /api/galaxy/getinfo/universe
Get Universe (SGU) gate information and addresses.

**Response**: Similar format with Universe-specific data
```json
{
    "galaxy_name": "Universe",
    "galaxy_type": "SGU",
    "symbol_count": 36,
    "chevron_count": 9,
    "addresses": [...]
}
```

**Implementation**: Returns 12 pre-programmed addresses from `UniverseGate.hpp`. Uses 9-symbol addresses (no point of origin).

---

## OTA API

### POST /ota/upload
Upload firmware binary for over-the-air update.

**Request**: Multipart form data with binary file

**Content-Type**: `multipart/form-data`

**Response**:
```json
{
    "success": true,
    "message": "Firmware uploaded successfully. Reboot required."
}
```

**Behavior**:
- Validates firmware image with `esp_ota_begin()`
- Writes to inactive OTA partition via `esp_ota_write()`
- Validates with `esp_ota_end()`
- Sets boot partition with `esp_ota_set_boot_partition()`
- **Manual reboot required** (auto-reboot not yet implemented, see TODO in OTA_API.cpp:83)

**Limitations**:
- Maximum file size: ~1.9 MB (OTA partition size)
- No progress reporting during upload
- No rollback mechanism on boot failure (planned)

---

## Disabled/Planned Endpoints

### GET /api/getfangatelist/milkyway
**Status**: Commented out in code (SystemAPI.cpp:42-48)

**Reason**: "TODO: Not ready" - Fan gate list API is not yet stable

**Implementation Note**: HttpClient.cpp fetches data from external service (thestargateproject.com), but endpoint is disabled until thread-safety and reliability issues are resolved.

---

## Endpoints NOT Implemented

The following endpoints were documented previously but do **NOT** exist in the actual implementation:

- ❌ `/api/system/status` (use `/api/getstatus` instead)
- ❌ `/api/system/info` (use `/api/getsysinfo` instead)
- ❌ `/api/system/reboot` (not implemented)
- ❌ `/api/gate/dial` (use `/api/control/dialaddress` instead)
- ❌ `/api/gate/dial_address` (not implemented)
- ❌ `/api/gate/home` (use `/api/control/autohome` instead)
- ❌ `/api/gate/calibrate` (use `/api/control/autocalibrate` instead)
- ❌ `/api/gate/stop` (use `/api/control/abort` instead)
- ❌ `/api/gate/close_wormhole` (use `/api/control/abort` instead)
- ❌ `/api/ring/status` (check `is_ring_connected` in `/api/getstatus`)
- ❌ `/api/ring/animation` (use `/api/ringcontrol/testanimate`)
- ❌ `/api/ring/symbols` (not implemented as REST endpoint, use BLE directly)
- ❌ `/api/settings` (use `/api/settingsjson`)
- ❌ `/api/settings/update` (not implemented)
- ❌ `/api/settings/export` (use `/api/settingsjson`)
- ❌ `/api/settings/import` (partial - use `/api/settingsjson` POST)
- ❌ `/api/galaxy/list` (not implemented)
- ❌ `/api/galaxy/addresses` (use galaxy-specific endpoints)
- ❌ `/api/ota/status` (not implemented)
- ❌ **WebSocket API** (`/ws`) - **NOT IMPLEMENTED**. Use polling of `/api/getstatus` instead.

---

## Error Responses

Most endpoints return HTTP status codes with JSON error messages on failure:

**500 Internal Server Error**:
```json
{
    "error": "Operation failed",
    "details": "Homing timeout after 30 seconds"
}
```

**Note**: Error format varies by endpoint. Some return simple text responses instead of JSON.

---

## API Usage Notes

### Polling Recommendations
- **Status updates**: Poll `/api/getstatus` every 500ms during operations (web interface standard)
- **System info**: Poll `/api/getsysinfo` once on page load
- **Ring connection**: Monitor `is_ring_connected` in status response

### Timeouts
- **Homing/Calibration**: 30 seconds per operation
- **Dial sequence**: 30 seconds per symbol movement (up to 7 × 30 = 210 seconds total for full address)
- **Wormhole duration**: 5 minutes (300 seconds) before automatic shutdown
- **Wormhole refresh**: 40ms (25Hz) with error handling for power instability

### Concurrent Requests
- The gate controller processes commands sequentially via FreeRTOS queue
- Multiple dial requests will be queued and executed in order
- Abort command (`/api/control/abort`) interrupts current operation

### Thread Safety
- All API handlers are thread-safe with FreeRTOS task protection
- HttpClient uses mutex protection for fan gate list access
- BLE operations are serialized through NimBLE stack

---

## Related Documentation

- [Architecture](architecture.md) - System overview and error handling patterns
- [BLE Protocol](ble-protocol.md) - Ring communication details
- [Control Page](control-page.md) - Web interface implementation
- [Homing Algorithm](homing-algorithm.md) - Homing procedure with timeout details
- [Dial Algorithm](dial-algorithm.md) - Symbol positioning math with error handling

---

## External API Integration

### Fan Gates Network (Planned)

The system plans to integrate with thestargateproject.com API for community features:

**External Endpoint** (not part of this device):
```
GET https://api.thestargateproject.com/get_fan_gates.php?galaxy=Milky%20Way
```

**Local Integration**: Currently disabled (`/api/getfangatelist/milkyway` commented out)

---

**Last Updated**: 2026-01-30
**API Version**: 1.0
**ESP-IDF Version**: 5.5.1
**Firmware**: pinky-board implementation
