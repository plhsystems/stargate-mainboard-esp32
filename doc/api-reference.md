# Stargate REST API Reference

## Overview

The Stargate mainboard exposes a comprehensive HTTP REST API on port 80 for control and monitoring. All endpoints accept and return JSON data (except for binary uploads).

**Base URL**: `http://stargate.local/` or `http://<IP_ADDRESS>/`

## Authentication

Currently, the API does not require authentication. All endpoints are publicly accessible on the local network.

## General Response Format

### Success Response
```json
{
    "status": "success",
    "data": { ... }
}
```

###Error Response
```json
{
    "status": "error",
    "message": "Error description",
    "code": 400
}
```

---

## System API

### GET /api/system/status
Get current system status and gate state.

**Response**:
```json
{
    "status": "success",
    "data": {
        "gate_state": "idle",
        "is_dialing": false,
        "current_chevron": 0,
        "ring_position": 0,
        "steps_per_rotation": 7680,
        "home_sensor": true,
        "uptime_ms": 123456,
        "free_heap": 234567,
        "wifi_rssi": -45
    }
}
```

**Gate States**:
- `idle`: Ready for commands
- `homing`: Executing homing sequence
- `calibrating`: Auto-calibration in progress
- `dialing`: Dialing an address
- `wormhole_active`: Wormhole is open
- `error`: Error state

---

### GET /api/system/info
Get system information and firmware version.

**Response**:
```json
{
    "status": "success",
    "data": {
        "firmware_version": "1.0.0",
        "hardware_revision": "pinky-board",
        "idf_version": "v5.3.1",
        "chip_model": "ESP32-S3",
        "chip_revision": 0,
        "cpu_freq_mhz": 240,
        "flash_size_mb": 16,
        "mac_address": "AA:BB:CC:DD:EE:FF"
    }
}
```

---

### POST /api/system/reboot
Reboot the system.

**Request**: Empty body

**Response**:
```json
{
    "status": "success",
    "message": "System rebooting..."
}
```

---

## Gate Control API

### POST /api/gate/dial
Dial a Stargate address.

**Request**:
```json
{
    "address": [12, 37, 24, 1, 32, 10, 1],
    "galaxy": "milky_way"
}
```

**Parameters**:
- `address`: Array of 7 symbol numbers (1-39 for Milky Way, 1-36 for others)
- `galaxy`: Gate type - `"milky_way"`, `"pegasus"`, `"atlantis"`, `"universe"`

**Response**:
```json
{
    "status": "success",
    "message": "Dialing address",
    "estimated_duration_ms": 45000
}
```

---

### POST /api/gate/dial_address
Dial using a named address from the address book.

**Request**:
```json
{
    "address_name": "Earth"
}
```

**Response**:
```json
{
    "status": "success",
    "message": "Dialing Earth",
    "address": [27, 7, 15, 32, 12, 30, 1]
}
```

---

### POST /api/gate/home
Execute homing sequence to calibrate ring position.

**Request**: Empty body

**Response**:
```json
{
    "status": "success",
    "message": "Homing started",
    "estimated_duration_ms": 10000
}
```

**Behavior**:
- Rotates ring until hall sensor is triggered
- Centers ring at home position
- Updates zero position reference

---

### POST /api/gate/calibrate
Execute auto-calibration to determine steps per rotation.

**Request**: Empty body

**Response**:
```json
{
    "status": "success",
    "message": "Calibration started",
    "estimated_duration_ms": 20000
}
```

**Behavior**:
- Performs full rotation
- Counts steps between home sensor triggers
- Saves result to NVS

---

### POST /api/gate/stop
Emergency stop - immediately halt all gate operations.

**Request**: Empty body

**Response**:
```json
{
    "status": "success",
    "message": "Gate stopped"
}
```

**Behavior**:
- Stops stepper motor
- Powers down servo
- Clears command queue
- Closes wormhole if active

---

### POST /api/gate/close_wormhole
Close an active wormhole.

**Request**: Empty body

**Response**:
```json
{
    "status": "success",
    "message": "Wormhole closing"
}
```

---

## Ring Control API

### GET /api/ring/status
Get BLE connection status with ring device.

**Response**:
```json
{
    "status": "success",
    "data": {
        "connected": true,
        "rssi": -45,
        "last_heartbeat_ms": 1234567890,
        "connection_uptime_ms": 300000
    }
}
```

---

### POST /api/ring/animation
Send animation command to ring.

**Request**:
```json
{
    "animation": "fade_in"
}
```

**Animation Types**:
- `"fade_in"`: Fade chevron LEDs in
- `"fade_out"`: Fade chevron LEDs out
- `"fade_in_fade_out"`: Fade in then out
- `"locked"`: Keep chevron lit
- `"invalid"`: Error state (rapid flash)

**Response**:
```json
{
    "status": "success",
    "message": "Animation sent"
}
```

---

### POST /api/ring/symbols
Set specific symbols to light up on ring.

**Request**:
```json
{
    "symbols": [1, 12, 24, 36]
}
```

**Response**:
```json
{
    "status": "success",
    "message": "Symbols updated"
}
```

---

### POST /api/ring/power_off
Power off the ring device.

**Request**: Empty body

**Response**:
```json
{
    "status": "success",
    "message": "Ring power off command sent"
}
```

---

## Sound Effects API

### GET /api/sound/list
List available sound effects.

**Response**:
```json
{
    "status": "success",
    "data": {
        "sounds": [
            {"id": 1, "name": "chevron_lock", "duration_ms": 2000},
            {"id": 2, "name": "wormhole_open", "duration_ms": 5000},
            {"id": 3, "name": "wormhole_close", "duration_ms": 3000}
        ]
    }
}
```

---

### POST /api/sound/play
Play a sound effect.

**Request**:
```json
{
    "sound_id": 1,
    "volume": 0.8
}
```

**Parameters**:
- `sound_id`: ID from sound list
- `volume`: Volume level (0.0 - 1.0)

**Response**:
```json
{
    "status": "success",
    "message": "Playing sound"
}
```

---

## Settings API

### GET /api/settings
Get current configuration.

**Response**:
```json
{
    "status": "success",
    "data": {
        "wifi_ssid": "MyNetwork",
        "gate_galaxy_type": "milky_way",
        "steps_per_rotation": 7680,
        "chevron_count": 9,
        "wormhole_brightness": 0.8,
        "ring_auto_off_timeout_ms": 30000
    }
}
```

---

### POST /api/settings/update
Update configuration settings.

**Request**:
```json
{
    "gate_galaxy_type": "pegasus",
    "wormhole_brightness": 0.9
}
```

**Response**:
```json
{
    "status": "success",
    "message": "Settings updated"
}
```

**Note**: Some settings require reboot to take effect.

---

### GET /api/settings/export
Export configuration as JSON file.

**Response**: JSON file download with all settings

---

### POST /api/settings/import
Import configuration from JSON file.

**Request**: Multipart form data with JSON file

**Response**:
```json
{
    "status": "success",
    "message": "Settings imported, reboot required"
}
```

---

## Galaxy Information API

### GET /api/galaxy/list
List all supported galaxy types.

**Response**:
```json
{
    "status": "success",
    "data": {
        "galaxies": [
            {
                "id": "milky_way",
                "name": "Milky Way",
                "symbol_count": 39,
                "chevron_count": 9,
                "spacing": "equal"
            },
            {
                "id": "pegasus",
                "name": "Pegasus",
                "symbol_count": 36,
                "chevron_count": 9,
                "spacing": "equal"
            },
            {
                "id": "universe",
                "name": "Universe",
                "symbol_count": 36,
                "chevron_count": 9,
                "spacing": "unequal"
            }
        ]
    }
}
```

---

### GET /api/galaxy/addresses
Get address book for current galaxy.

**Response**:
```json
{
    "status": "success",
    "data": {
        "galaxy": "milky_way",
        "addresses": [
            {
                "name": "Earth",
                "symbols": [27, 7, 15, 32, 12, 30, 1],
                "description": "Point of origin: Earth"
            },
            {
                "name": "Abydos",
                "symbols": [26, 6, 14, 31, 11, 29, 1],
                "description": "First mission"
            }
        ]
    }
}
```

---

## OTA (Over-The-Air) Update API

### POST /api/ota/upload
Upload new firmware binary.

**Request**: Multipart form data with binary file

**Response**:
```json
{
    "status": "success",
    "message": "Firmware uploaded, rebooting..."
}
```

**Process**:
1. Upload firmware binary
2. Verify firmware signature
3. Write to OTA partition
4. Set boot partition
5. Reboot system

---

### GET /api/ota/status
Get OTA update status.

**Response**:
```json
{
    "status": "success",
    "data": {
        "update_available": false,
        "current_version": "1.0.0",
        "ota_partition": "ota_0",
        "next_partition": "ota_1"
    }
}
```

---

## WebSocket API (Real-Time Updates)

### Connection
**URL**: `ws://stargate.local/ws`

**Message Format**:
```json
{
    "type": "status_update",
    "data": {
        "gate_state": "dialing",
        "current_chevron": 3,
        "ring_position": 2340
    }
}
```

**Event Types**:
- `status_update`: Periodic status updates
- `chevron_locked`: Chevron lock event
- `wormhole_opened`: Wormhole establishment
- `wormhole_closed`: Wormhole shutdown
- `error`: Error event

---

## Error Codes

| Code | Description |
|------|-------------|
| 400 | Bad Request - Invalid parameters |
| 404 | Not Found - Endpoint doesn't exist |
| 409 | Conflict - Gate busy with another operation |
| 500 | Internal Server Error |
| 503 | Service Unavailable - Hardware error |

---

## Rate Limiting

- **Default**: 60 requests per minute per IP
- **Burst**: Up to 10 requests per second
- **Headers**:
  - `X-RateLimit-Limit`: Maximum requests
  - `X-RateLimit-Remaining`: Remaining requests
  - `X-RateLimit-Reset`: Reset timestamp

---

## External API Integration

### Fan Gates Network
Some API from thestargateproject.com can enhance the user experience.

**Get Fan Gates List**:
```
GET https://api.thestargateproject.com/get_fan_gates.php
```

**Filter by Galaxy**:
```
GET https://api.thestargateproject.com/get_fan_gates.php?galaxy=Milky%20Way
```

**Response**:
```json
{
    "gates": [
        {
            "name": "BuildTheStargate HQ",
            "address": [12, 37, 24, 1, 32, 10, 1],
            "online": true,
            "location": "France"
        }
    ]
}
```

---

## Example Usage

### Python Example
```python
import requests

# Get status
response = requests.get('http://stargate.local/api/system/status')
status = response.json()
print(f"Gate state: {status['data']['gate_state']}")

# Dial Earth
dial_request = {
    "address": [27, 7, 15, 32, 12, 30, 1],
    "galaxy": "milky_way"
}
response = requests.post('http://stargate.local/api/gate/dial',
                        json=dial_request)
print(response.json()['message'])
```

### cURL Example
```bash
# Get system info
curl http://stargate.local/api/system/info

# Dial Abydos
curl -X POST http://stargate.local/api/gate/dial \
  -H "Content-Type: application/json" \
  -d '{"address": [26, 6, 14, 31, 11, 29, 1], "galaxy": "milky_way"}'

# Stop gate
curl -X POST http://stargate.local/api/gate/stop
```

---

## Related Documentation

- [Architecture](architecture.md) - System design overview
- [BLE Protocol](ble-protocol.md) - Ring communication details
- [Web Interface](webpage.md) - Web UI features
