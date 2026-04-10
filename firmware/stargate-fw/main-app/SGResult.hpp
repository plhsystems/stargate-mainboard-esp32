#pragma once

#include <iterator>

enum class SGResult
{
    OK = 0,

    Failed,
    Timeout,
    NotCalibrated,
    NotHomed,
    Cancelled,
    HardwareFailure,
    SensorFailure,
    MotorJammed,
    InvalidParameter,
    BLEDisconnected,
    BLETimeout,
    BLE_CharNotDiscovered,
    BLE_NotConnected,
    BLE_WriteFailed,
    Sound_InvalidFileID,
    Wormhole_PowerInstability,

    Count
};

// Array of text descriptions for each enum value
inline const char* const SGRESULT_TEXTS[] = {
    "OK",
    "Failed",
    "Timeout",
    "Calibration required",
    "Homing required",
    "Cancelled by user",
    "Hardware failure",
    "Sensor failure",
    "Motor jammed or stalled",
    "Invalid parameter",
    "BLE disconnected",
    "BLE timeout",
    "BLE characteristic not discovered",
    "BLE not connected",
    "BLE write failed",
    "Sound: invalid file ID",
    "Wormhole: power instability",
};

// Static assert to ensure the array size matches the enum count
static_assert( std::size(SGRESULT_TEXTS) == static_cast<size_t>(SGResult::Count), "SGResultText array size must match SGResult enum count");

// Helper function to get text for an enum value
inline const char* getSGResultText(SGResult result)
{
    return SGRESULT_TEXTS[static_cast<size_t>(result)];
}

#define SGRESULT_OK_OR_RETURN(__ret__) do { \
    if (SGResult::OK != (__ret__)) { \
        return __ret__; \
    } \
} while(0)
