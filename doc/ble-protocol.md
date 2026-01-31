# BLE Communication Protocol

## Overview

The Stargate system uses Bluetooth Low Energy (BLE) for communication between the main controller (ESP32-S3) and the ring controller (ESP32-WROOM). The main controller acts as a BLE Central (client), while the ring acts as a BLE Peripheral (server).

## Connection Architecture

```
┌──────────────────────────┐          ┌──────────────────────────┐
│   Main Controller        │          │   Ring Controller        │
│   (ESP32-S3)             │          │   (ESP32-WROOM)          │
│                          │          │                          │
│  ┌────────────────────┐  │          │  ┌────────────────────┐  │
│  │  RingBLEClient     │  │   BLE    │  │  BLE Server        │  │
│  │  (Central/Client)  │  │◄────────►│  │  (Peripheral)      │  │
│  │                    │  │          │  │                    │  │
│  │  • Scanning        │  │          │  │  • Advertisement   │  │
│  │  • Connecting      │  │          │  │  • GATT Services   │  │
│  │  • Writing Cmds    │  │          │  │  • Processing      │  │
│  └────────────────────┘  │          │  └────────────────────┘  │
└──────────────────────────┘          └──────────────────────────┘
```

## GATT Service Structure

### Service
- **UUID**: `12345678-1234-5678-1234-56789abcdef0`
- **Type**: Primary service
- **Purpose**: Main communication service

### Characteristic
- **UUID**: `12345678-1234-5678-1234-56789abcdef1`
- **Properties**: `WRITE` (with response)
- **Max Length**: 20 bytes
- **Purpose**: Command channel from main controller to ring

**Implementation Details**:
```c
// Service definition in ring-fw/main/ble_server.c
static const ble_uuid128_t service_uuid =
    BLE_UUID128_INIT(0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static const ble_uuid128_t char_uuid =
    BLE_UUID128_INIT(0xf1, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);
```

## SGUComm Protocol

### Message Format

All BLE messages follow the SGUComm protocol defined in `sgc-components/sgu-ringcomm/SGUComm.hpp`.

**Base Structure**:
```cpp
struct SGUCommHeader {
    uint8_t action;      // Action type (0-5)
    uint8_t length;      // Payload length
    uint8_t payload[];   // Variable-length payload
};
```

### Action Types

#### Action 0: Heartbeat
**Purpose**: Keep-alive ping to maintain connection
**Direction**: Main → Ring
**Frequency**: Every 1000ms
**Payload**: None

**Implementation**:
```cpp
// Main controller sends
RingBLEClient::SendHeartbeat();

// Ring receives and processes
// No response required
```

**BLE Frame**:
```
[0x00] [0x00]
  ^      ^
  |      └─ Length: 0 bytes
  └─ Action: Heartbeat
```

---

#### Action 1: Animation
**Purpose**: Trigger chevron lighting animation
**Direction**: Main → Ring
**Payload**: 1 byte (animation type)

**Animation Types** (EChevronAnimation enum in SGUComm.hpp):
```cpp
enum EChevronAnimation : uint8_t {
    Chevron_FadeIn = 0,         // Fade chevron LEDs in
    Chevron_FadeOut = 1,        // Fade chevron LEDs out
    Chevron_ErrorToWhite = 2,   // Error flash to white
    Chevron_ErrorToOff = 3,     // Error flash to off
    Chevron_AllSymbolsOn = 4,   // Turn all symbols on
    Chevron_PoweringOff = 5,    // Power-off animation
    Chevron_NoSymbols = 6       // Turn all symbols off
};
```

**Note**: `Chevron_FadeIn` is automatically sent when a successful BLE connection is established (UUID verification passed)

**Implementation**:
```cpp
// Main controller sends
RingBLEClient::SendAnimation(SGUCommNS::EChevronAnimation::FadeIn);
```

**BLE Frame**:
```
[0x01] [0x01] [0x00]
  ^      ^      ^
  |      |      └─ Payload: FadeIn animation
  |      └─ Length: 1 byte
  └─ Action: Animation
```

**Ring Behavior**:
- Executes LED animation on the ring
- Duration: Depends on animation type (~500-2000ms)
- Blocking: Completes before processing next command

---

#### Action 2: Set Symbols
**Purpose**: Set specific symbols to light up on ring
**Direction**: Main → Ring
**Payload**: 6 bytes (48-bit pattern, 1 bit per symbol)

**Bit Mapping**:
- Bit 0-47: Represents symbols 1-48
- Bit value 1: Symbol is lit
- Bit value 0: Symbol is dark

**Implementation**:
```cpp
// Main controller sends
uint8_t symbol_bits[6] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};
RingBLEClient::SendSetSymbols(symbol_bits);
// Lights up symbols 1-8
```

**BLE Frame**:
```
[0x02] [0x06] [B0] [B1] [B2] [B3] [B4] [B5]
  ^      ^     ^    ^    ^    ^    ^    ^
  |      |     └────┴────┴────┴────┴────┴─ 48 bits of symbol data
  |      └─ Length: 6 bytes
  └─ Action: Set Symbols
```

**Example**:
To light symbols 12, 24, 36:
```cpp
uint8_t symbols[6] = {0};
symbols[1] |= (1 << 3);  // Symbol 12 (bit 11)
symbols[2] |= (1 << 7);  // Symbol 24 (bit 23)
symbols[4] |= (1 << 3);  // Symbol 36 (bit 35)
```

---

#### Action 3: Power Off
**Purpose**: Put ring into sleep mode
**Direction**: Main → Ring
**Payload**: None

**Implementation**:
```cpp
// Main controller sends
RingBLEClient::SendPowerOff();
```

**BLE Frame**:
```
[0x03] [0x00]
  ^      ^
  |      └─ Length: 0 bytes
  └─ Action: Power Off
```

**Ring Behavior**:
- Turns off all LEDs
- Disconnects BLE
- Enters deep sleep mode
- Wakes on reset or timeout

---

#### Action 4: Light Symbol
**Purpose**: Light up a single symbol with specified brightness
**Direction**: Main → Ring
**Payload**: 2 bytes (symbol_index + level_pwm)

**Implementation**:
```cpp
// Main controller sends
RingBLEClient::SendLightUpSymbol(uint8_t symbol_index, uint8_t brightness = 255);
```

**BLE Frame**:
```
[0x04] [0x02] [0x17] [0xFF]
  ^      ^      ^      ^
  |      |      |      └─ Brightness (PWM level 0-255)
  |      |      └─ Symbol index 23 (0-indexed, range 0-47)
  |      └─ Length: 2 bytes
  └─ Action: Light Symbol
```

**Parameters**:
- **symbol_index**: Index of symbol to light (0-47 for 48-pixel ring)
- **level_pwm**: Brightness level (0 = off, 255 = full brightness)

**Ring Behavior**:
- Turns on specified symbol LED at given brightness
- All other symbols remain in current state
- Immediate effect (no animation)

**Example**:
```cpp
// Light symbol 12 at 50% brightness
RingBLEClient::SendLightUpSymbol(11, 128);
```

---

#### Action 5: Factory Mode
**Purpose**: Reset ring to factory defaults
**Direction**: Main → Ring
**Payload**: None

**Implementation**:
```cpp
// Main controller sends
RingBLEClient::SendGotoFactory();
```

**BLE Frame**:
```
[0x05] [0x00]
  ^      ^
  |      └─ Length: 0 bytes
  └─ Action: Factory Mode
```

**Ring Behavior**:
- Erases NVS configuration
- Resets to factory firmware
- Requires manual reboot

---

## Connection Management

### Scanning Phase
**Main Controller** (`RingBLEClient.cpp`):
```cpp
void StartScan() {
    ble_gap_disc_params_t disc_params = {
        .itvl = 0,          // Use default
        .window = 0,        // Use default
        .filter_policy = 0, // No filter
        .limited = 0,       // General discovery
        .passive = 0,       // Active scanning
        .filter_duplicates = 1
    };
    ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params,
                 GapEventHandler, NULL);
}
```

**Scan Parameters**:
- **Interval**: 5000ms between scans
- **Duration**: Continuous until connection
- **Filter**: Device name contains "StarGate" or "Ring"

### Connection Phase
**Connection Parameters**:
```cpp
struct ble_gap_conn_params {
    .scan_itvl = 0x0010,    // 10ms
    .scan_window = 0x0010,  // 10ms
    .itvl_min = 24,         // 30ms
    .itvl_max = 40,         // 50ms
    .latency = 0,
    .supervision_timeout = 256,  // 2.56s
    .min_ce_len = 0,
    .max_ce_len = 0
};
```

**Connection Flow**:
```
1. Main scans for "RingFW-BLE" device
   │
   ▼
2. Found device → Connect request
   │
   ▼
3. Connection established
   │
   ▼
4. Service discovery (UUID: 12345678...)
   │
   ▼
5. Characteristic discovery (UUID: 12345678...f1)
   │
   ▼
6. UUID verification (ensures correct characteristic)
   │
   ▼
7. Save characteristic handle
   │
   ▼
8. Send Chevron_FadeIn animation (connection indicator)
   │
   ▼
9. Start heartbeat timer (1000ms)
   │
   ▼
10. Ready for commands
```

**UUID Verification** (Recent Enhancement):
The characteristic discovery now includes UUID verification to ensure the correct characteristic is selected:
```cpp
// Check if this is the correct characteristic by comparing UUIDs
if (ble_uuid_cmp(&chr->uuid.u, &CHAR_UUID.u) == 0) {
    ESP_LOGI(TAG, "UUID matches! Storing handle=%d", chr->val_handle);
    m_char_value_handle = chr->val_handle;
    m_char_discovered = true;

    // Send connection animation to indicate successful pairing
    client->SendAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeIn);
}
```

This prevents accidentally using a wrong characteristic if multiple WRITE characteristics exist on the device.

### Disconnection Handling
**Auto-Reconnect**:
- Enabled by default
- Retry interval: 5000ms
- Infinite retries

**Disconnect Reasons**:
- Ring powered off (Action 3)
- Connection timeout (supervision timeout exceeded)
- Out of range
- Ring reset

---

## Implementation Details

### Main Controller (stargate-fw)

**File**: `firmware/stargate-fw/main-app/Ring/RingBLEClient.cpp`

**Key Functions**:
```cpp
class RingBLEClient {
public:
    // Initialization
    void Init();
    void Start();

    // Connection status
    bool GetIsConnected();

    // Command methods
    void SendHeartbeat();
    void SendAnimation(SGUCommNS::EChevronAnimation animation);
    void SendSetSymbols(const uint8_t symbol_bits[6]);
    void SendPowerOff();
    void SendLightUpSymbol(uint8_t symbol_index);
    void SendGotoFactory();

private:
    // BLE callbacks
    static int GapEventHandler(struct ble_gap_event *event, void *arg);

    // Write to characteristic
    bool WriteCharacteristic(const uint8_t* data, uint16_t len);

    // State
    uint16_t m_conn_handle;
    uint16_t m_char_value_handle;
    bool m_is_connected;
};
```

**Heartbeat Task**:
```cpp
void TaskRunning(void* arg) {
    while (true) {
        if (GetIsConnected()) {
            SendHeartbeat();
        } else {
            StartScan();
        }
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_INTERVAL_MS));
    }
}
```

### Ring Controller (ring-fw)

**File**: `firmware/ring-fw/main/ble_server.c`

**Key Functions**:
```c
// BLE initialization
int ble_server_init(const ble_action_callbacks_t *callbacks);
void ble_server_deinit(void);

// GATT characteristic callback
static int gatt_characteristic_callback(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt,
    void *arg);
```

**Message Processing**:
```c
// In main.cpp
static void SGUBRHeartbeatHandler() {
    ESP_LOGI(TAG, "BLE Heartbeat received");
    // Reset auto-off timer
}

static void SGUBRAnimationHandler(const SChevronAnimationArg* psArg) {
    ESP_LOGI(TAG, "BLE Animation received: %u", psArg->u8Animation);
    // Execute LED animation
}

static void SGUBRUpdateLightHandler(const SUpdateLightArg* psArg) {
    ESP_LOGI(TAG, "BLE Update light received. Lights: %u",
             (unsigned int)psArg->u8LightCount);
    // Update LED colors
}
```

---

## Power Management

### Ring Auto-Sleep
**Default Timeout**: 30 seconds of inactivity
**Configuration**: Stored in NVS (`ring.auto_off_timeout`)

**Inactivity Detection**:
- Last received command timestamp
- Heartbeat resets timer
- Any action resets timer

**Sleep Entry**:
```c
if (millis() - last_command_time > auto_off_timeout) {
    // Turn off LEDs
    // Disconnect BLE
    // Enter deep sleep
    esp_deep_sleep_start();
}
```

**Wake Sources**:
- Hardware reset button
- GPIO wake-up (configurable)

---

## Error Handling

### Connection Errors
**Timeout**:
- Supervision timeout: 2.56 seconds
- Action: Auto-reconnect

**Write Failure**:
- Retry count: 3 attempts
- Retry interval: 100ms
- Fallback: Mark connection as lost

### Invalid Commands
**Unknown Action**:
```c
if (action > 5) {
    ESP_LOGE(TAG, "Invalid action: %u", action);
    return;  // Ignore command
}
```

**Payload Length Mismatch**:
```c
if (expected_length != received_length) {
    ESP_LOGE(TAG, "Invalid payload length");
    return;  // Ignore command
}
```

---

## Performance Characteristics

### Latency
- **Command to LED Update**: 20-50ms
- **Connection Setup**: 2-5 seconds
- **Service Discovery**: 500-1000ms

### Throughput
- **Max Commands/sec**: ~20 (limited by heartbeat and processing)
- **BLE MTU**: 23-512 bytes (negotiated)
- **Effective Rate**: ~2 KB/s

### Reliability
- **Packet Loss**: <1% with good signal
- **Reconnection Time**: 5 seconds
- **Heartbeat Timeout**: 3 missed heartbeats = disconnection

---

## Debugging

### Log Tags
**Main Controller**:
```cpp
#define TAG "RingBLEClient"
ESP_LOGI(TAG, "Connected to ring");
ESP_LOGW(TAG, "Connection lost, retrying...");
ESP_LOGE(TAG, "Failed to write characteristic");
```

**Ring Controller**:
```c
#define TAG "BLE_SERVER"
ESP_LOGI(TAG, "BLE server initialized");
ESP_LOGD(TAG, "Received action: %u", action);
```

### Connection Status
**Check via Web API**:
```bash
curl http://stargate.local/api/ring/status
```

**Response**:
```json
{
    "connected": true,
    "rssi": -45,
    "last_heartbeat": 1234567890
}
```

---

## Security Considerations

### Current Implementation
- **Pairing**: Not required (open connection)
- **Encryption**: BLE Link Layer encryption (AES-128)
- **Authentication**: None (device name filtering only)

### Recommendations for Production
1. **Enable BLE pairing** with PIN or passkey
2. **Whitelist MAC addresses** for known rings
3. **Implement command authentication** (HMAC)
4. **Rate limiting** to prevent DoS attacks

---

## Future Enhancements

1. **Bidirectional Communication**: Ring → Main status reports
2. **OTA Updates**: Firmware updates via BLE
3. **Extended MTU**: Support for larger payloads
4. **Multiple Rings**: Support for auxiliary ring devices
5. **BLE Mesh**: Multiple gates in mesh network

---

## Related Documentation

- [Architecture](architecture.md) - Overall system design
- [SGUComm Protocol](../firmware/sgc-components/sgu-ringcomm/SGUComm.hpp) - Protocol header file
- [RingBLEClient Implementation](../firmware/stargate-fw/main-app/Ring/RingBLEClient.cpp)
- [BLE Server Implementation](../firmware/ring-fw/main/ble_server.c)
