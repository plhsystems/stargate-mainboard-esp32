#pragma once

#include "../SGResult.hpp"

/**
 * @brief Result type for operations that can fail
 *
 * This provides a type-safe way to return either a success value or an error code.
 * For void operations, use Result<void> which only carries the error state.
 *
 * Example usage:
 * @code
 * Result<int32_t> getPosition() {
 *     if (!calibrated) return SGResult::NotCalibrated;
 *     return Result<int32_t>::success(position);
 * }
 *
 * auto result = getPosition();
 * if (result.ok()) {
 *     int32_t pos = result.value();
 * } else {
 *     ESP_LOGE(TAG, "Error: %s", getSGResultText(result.error()));
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
        r.m_error = SGResult::OK;
        r.m_value = value;
        return r;
    }

    // Error constructor
    Result(SGResult error) : m_error(error), m_value{} {}

    // Default constructor (error state)
    Result() : m_error(SGResult::HardwareFailure), m_value{} {}

    /// Check if operation succeeded
    bool ok() const { return SGResult::OK == m_error; }

    /// Check if operation failed
    bool failed() const { return SGResult::OK != m_error; }

    /// Get the error code
    SGResult error() const { return m_error; }

    /// Get the error message string
    const char* errorString() const { return getSGResultText(m_error); }

    /// Get the value (only valid if ok() returns true)
    const T& value() const { return m_value; }
    T& value() { return m_value; }

    /// Get value or default if error
    T valueOr(const T& default_value) const
    {
        return ok() ? m_value : default_value;
    }

    // Implicit conversion to bool for convenience
    explicit operator bool() const { return ok(); }

private:
    SGResult m_error;
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
        r.m_error = SGResult::OK;
        return r;
    }

    // Error constructor
    Result(SGResult error) : m_error(error) {}

    // Default constructor (success state for convenience)
    Result() : m_error(SGResult::OK) {}

    /// Check if operation succeeded
    bool ok() const { return SGResult::OK == m_error; }

    /// Check if operation failed
    bool failed() const { return SGResult::OK != m_error; }

    /// Get the error code
    SGResult error() const { return m_error; }

    /// Get the error message string
    const char* errorString() const { return getSGResultText(m_error); }

    // Implicit conversion to bool for convenience
    explicit operator bool() const { return ok(); }

private:
    SGResult m_error;
};

// Convenience alias
using VoidResult = Result<void>;
