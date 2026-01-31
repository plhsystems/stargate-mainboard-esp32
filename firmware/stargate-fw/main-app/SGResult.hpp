#pragma once

#include <iterator>

enum class SGResult
{
    Ok = 0,             ///< Operation completed successfully

    Timeout,            ///< Operation timed out waiting for hardware response
    SensorFailure,      ///< Sensor not responding or returning invalid data
    MotorJammed,        ///< Motor unable to complete movement (stall detected)
    NotCalibrated,      ///< Operation requires calibration that hasn't been done
    NotHomed,           ///< Operation requires homing that hasn't been done
    Cancelled,          ///< Operation was cancelled by user request
    InvalidParameter,   ///< Invalid parameter passed to function
    HardwareFailure,    ///< General hardware failure
    BLEDisconnected,    ///< BLE connection lost during operation
    BLETimeout,         ///< BLE operation timed out

    Count
};

// Array of text descriptions for each enum value
const char* SGRESULT_TEXTS[] =
{
    "Ok",

    "Operation timed out",
    "Sensor failure",
    "Motor jammed or stalled",
    "Calibration required",
    "Homing required",
    "Cancelled by user",
    "Invalid parameter",
    "Hardware failure",
    "BLE disconnected",
    "BLE timeout"
};

// Static assert to ensure the array size matches the enum count
static_assert( std::size(SGRESULT_TEXTS) == static_cast<size_t>(SGResult::Count), "SGResultText array size must match SGResult enum count");

// Helper function to get text for an enum value
const char* GetSGResultText(SGResult result)
{
    return SGRESULT_TEXTS[static_cast<size_t>(result)];
}

#define SGRESULT_OK_OR_RETURN(__ret__) do { \
    if ( SGResult::Ok != (__ret__) ) {
        return __ret__;
    }
} while(0)