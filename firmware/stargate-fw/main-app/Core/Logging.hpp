#pragma once

#include "esp_log.h"

/**
 * @brief Structured Logging Guidelines
 *
 * This header provides documentation and macros for consistent logging
 * across all components in the stargate firmware.
 *
 * ## Log Tag Convention
 *
 * Each source file should define a TAG at the top:
 * ```cpp
 * static const char* TAG = "ComponentName";
 * ```
 *
 * Use short, descriptive names (max 15 chars for alignment):
 * - "GateControl"   - Gate state machine and motor control
 * - "RingBLEClient" - BLE communication with ring
 * - "WebServer"     - HTTP server and API
 * - "WifiMgr"       - WiFi station and soft AP
 * - "Settings"      - NVS configuration storage
 * - "Wormhole"      - LED animation effects
 * - "HAL"           - Hardware abstraction layer
 * - "StateMachine"  - Formal state machine transitions
 *
 * ## Log Levels
 *
 * Use appropriate log levels:
 * - ESP_LOGE: Errors that prevent operation (hardware failures, timeouts)
 * - ESP_LOGW: Warnings about unexpected but recoverable situations
 * - ESP_LOGI: Important state changes and milestones
 * - ESP_LOGD: Detailed debugging info (disabled in release)
 * - ESP_LOGV: Verbose tracing (disabled by default)
 *
 * ## Structured Log Messages
 *
 * Include relevant context in log messages:
 * - State transitions: "State: Idle -> Homing"
 * - Operations: "Starting calibration, timeout=30000ms"
 * - Results: "Calibration complete, steps_per_rotation=1600"
 * - Errors: "Timeout waiting for home sensor, elapsed=30000ms"
 *
 * ## Conditional Compilation
 *
 * For performance-critical code, use compile-time checks:
 * ```cpp
 * #if CONFIG_LOG_DEFAULT_LEVEL >= ESP_LOG_DEBUG
 *     ESP_LOGD(TAG, "Debug info: %d", value);
 * #endif
 * ```
 */

// Macro to log state transitions with standardized format
#define LOG_STATE_TRANSITION(tag, from_state, to_state, event) \
    ESP_LOGI(tag, "State: %s -> %s (event: %s)", \
             (from_state), (to_state), (event))

// Macro to log operation start with parameters
#define LOG_OPERATION_START(tag, operation, ...) \
    ESP_LOGI(tag, "Starting %s" __VA_OPT__(", ") __VA_ARGS__, (operation))

// Macro to log operation success
#define LOG_OPERATION_SUCCESS(tag, operation) \
    ESP_LOGI(tag, "%s completed successfully", (operation))

// Macro to log operation failure with error code
#define LOG_OPERATION_FAILED(tag, operation, error_str) \
    ESP_LOGE(tag, "%s failed: %s", (operation), (error_str))

// Macro to log timeout errors
#define LOG_TIMEOUT(tag, operation, timeout_ms) \
    ESP_LOGE(tag, "%s timed out after %lu ms", (operation), (unsigned long)(timeout_ms))

// Macro to log hardware values (useful for debugging)
#define LOG_HW_VALUE(tag, name, value) \
    ESP_LOGD(tag, "%s = %d", (name), (int)(value))

// Macro to log BLE events
#define LOG_BLE_EVENT(tag, event_name, status) \
    ESP_LOGI(tag, "BLE %s: status=%d", (event_name), (int)(status))
