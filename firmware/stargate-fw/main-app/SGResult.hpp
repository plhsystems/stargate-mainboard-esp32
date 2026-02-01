#pragma once

#include <iterator>

enum class SGResult
{
    // General/Common
    OK = 0,
    Failed,
    Cancelled,
    Timeout,
    NotInitialized,
    InvalidParameter,

    // WiFi Manager
    WiFi_InterfaceNull,
    WiFi_NotConnected,
    WiFi_InvalidPassword,

    // Wormhole
    Wormhole_RefreshFailed,
    Wormhole_PowerInstability,
    Wormhole_LEDDriverError,

    // BLE Ring Client
    BLE_NotConnected,
    BLE_CharNotDiscovered,
    BLE_WriteFailed,
    BLE_DeviceNotFound,
    BLE_ConnectionFailed,

    // Sound/Audio
    Sound_InvalidFileID,
    Sound_FileNotFound,
    Sound_HardwareError,

    // Settings/Configuration
    Settings_ParseError,
    Settings_ValidationError,
    Settings_ImportFailed,
    Settings_SaveFailed,

    // Hardware Abstraction Layer
    HAL_SensorError,
    HAL_MotorTimeout,
    HAL_HomeSensorInactive,
    HAL_StepperError,
    HAL_ServoError,
    HAL_PowerError,

    // Gate Control
    Gate_NotCalibrated,
    Gate_NotHomed,
    Gate_CalibrationFailed,
    Gate_HomingFailed,
    Gate_DialFailed,
    Gate_DialTimeout,
    Gate_InvalidAddress,
    Gate_ClampError,

    Count
};

// Array of text descriptions for each enum value
static const char* SGRESULT_TEXTS[] = 
{
    // General/Common
    "OK",
    "Failed",
    "Cancelled",
    "Timeout",
    "Not initialized",
    "Invalid parameter",

    // WiFi Manager
    "WiFi interface is NULL",
    "WiFi not connected",
    "WiFi password validation failed",

    // Wormhole
    "Wormhole LED refresh failed",
    "Wormhole power instability detected",
    "Wormhole LED driver error",

    // BLE Ring Client
    "BLE not connected to ring",
    "BLE characteristic not discovered",
    "BLE write operation failed",
    "BLE ring device not found",
    "BLE connection failed",

    // Sound/Audio
    "Invalid sound file ID",
    "Sound file not found",
    "Audio hardware error",

    // Settings/Configuration
    "Settings JSON parse error",
    "Settings validation error",
    "Settings import failed",
    "Settings save to NVS failed",

    // Hardware Abstraction Layer
    "HAL sensor read error",
    "HAL motor movement timeout",
    "HAL home sensor inactive",
    "HAL stepper motor error",
    "HAL servo motor error",
    "HAL power subsystem error",

    // Gate Control
    "Gate needs calibration",
    "Gate homing required",
    "Gate calibration failed",
    "Gate homing failed",
    "Gate dial sequence failed",
    "Gate dial sequence timeout",
    "Invalid gate address",
    "Gate chevron clamp error"
};

// Static assert to ensure the array size matches the enum count
static_assert( std::size(SGRESULT_TEXTS) == static_cast<size_t>(SGResult::Count), "SGResultText array size must match SGResult enum count");

// Helper function to get text for an enum value
inline const char* GetSGResultText(SGResult result)
{
    size_t index = static_cast<size_t>(result);
    if (index >= static_cast<size_t>(SGResult::Count)) {
        return "Unknown error";
    }
    return SGRESULT_TEXTS[index];
}

#define SGRESULT_OK_OR_RETURN(__ret__) do { \
    if ( SGResult::OK != (__ret__) ) { \
        return __ret__; \
    } \
} while(0)
