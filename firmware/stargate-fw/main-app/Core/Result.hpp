#pragma once

#include <cstdint>

/**
 * @brief Hardware operation error codes
 *
 * These error codes provide detailed information about why a hardware
 * operation failed, enabling better diagnostics and error handling.
 */
enum class HWError : uint8_t
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

/**
 * @brief Get human-readable string for an error code
 */
inline const char* HWErrorToString(HWError error)
{
    switch (error)
    {
        case HWError::Ok:               return "Success";
        case HWError::Timeout:          return "Operation timed out";
        case HWError::SensorFailure:    return "Sensor failure";
        case HWError::MotorJammed:      return "Motor jammed or stalled";
        case HWError::NotCalibrated:    return "Calibration required";
        case HWError::NotHomed:         return "Homing required";
        case HWError::Cancelled:        return "Cancelled by user";
        case HWError::InvalidParameter: return "Invalid parameter";
        case HWError::HardwareFailure:  return "Hardware failure";
        case HWError::BLEDisconnected:  return "BLE disconnected";
        case HWError::BLETimeout:       return "BLE timeout";
        default:                        return "Unknown error";
    }
}

/**
 * @brief Result type for operations that can fail
 *
 * This provides a type-safe way to return either a success value or an error code.
 * For void operations, use Result<void> which only carries the error state.
 *
 * Example usage:
 * @code
 * Result<int32_t> GetPosition() {
 *     if (!calibrated) return HWError::NotCalibrated;
 *     return Result<int32_t>::success(position);
 * }
 *
 * auto result = GetPosition();
 * if (result.ok()) {
 *     int32_t pos = result.value();
 * } else {
 *     ESP_LOGE(TAG, "Error: %s", HWErrorToString(result.error()));
 * }
 * @endcode
 */
template<typename T = void>
class Result
{
public:
    // Success constructor with value
    static Result success(const T& value)
    {
        Result r;
        r.m_error = HWError::Ok;
        r.m_value = value;
        return r;
    }

    // Error constructor
    Result(HWError error) : m_error(error), m_value{} {}

    // Default constructor (error state)
    Result() : m_error(HWError::HardwareFailure), m_value{} {}

    /// Check if operation succeeded
    bool ok() const { return m_error == HWError::Ok; }

    /// Check if operation failed
    bool failed() const { return m_error != HWError::Ok; }

    /// Get the error code
    HWError error() const { return m_error; }

    /// Get the error message string
    const char* errorString() const { return HWErrorToString(m_error); }

    /// Get the value (only valid if ok() returns true)
    const T& value() const { return m_value; }
    T& value() { return m_value; }

    /// Get value or default if error
    T valueOr(const T& defaultValue) const
    {
        return ok() ? m_value : defaultValue;
    }

    // Implicit conversion to bool for convenience
    explicit operator bool() const { return ok(); }

private:
    HWError m_error;
    T m_value;
};

/**
 * @brief Specialization for void results (no value, just success/error)
 */
template<>
class Result<void>
{
public:
    // Success factory
    static Result success()
    {
        Result r;
        r.m_error = HWError::Ok;
        return r;
    }

    // Error constructor
    Result(HWError error) : m_error(error) {}

    // Default constructor (success state for convenience)
    Result() : m_error(HWError::Ok) {}

    /// Check if operation succeeded
    bool ok() const { return m_error == HWError::Ok; }

    /// Check if operation failed
    bool failed() const { return m_error != HWError::Ok; }

    /// Get the error code
    HWError error() const { return m_error; }

    /// Get the error message string
    const char* errorString() const { return HWErrorToString(m_error); }

    // Implicit conversion to bool for convenience
    explicit operator bool() const { return ok(); }

private:
    HWError m_error;
};

// Convenience alias
using VoidResult = Result<void>;
