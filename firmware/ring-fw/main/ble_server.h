#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <stdint.h>

#ifdef __cplusplus
#include "SGUComm.hpp"
typedef SGUCommNS::EChevronAnimation EChevronAnimation;
extern "C" {
#else
// For C code, use uint8_t
typedef uint8_t EChevronAnimation;
#endif

/**
 * @brief BLE action types
 */
typedef enum {
    BLE_ACTION_HEARTBEAT = 0,        // Heartbeat
    BLE_ACTION_ANIMATION = 1,        // Execute ring animation
    BLE_ACTION_SET_SYMBOLS = 2,      // Set lighted symbols (48 bits)
    BLE_ACTION_POWER_OFF = 3,        // Power off the ring
    BLE_ACTION_LIGHT_SYMBOL = 4,     // Light up a single symbol
    BLE_ACTION_GOTO_FACTORY = 5,     // Go to factory mode
} ble_action_type_t;

/**
 * @brief Callback for action 0 - Heartbeat
 */
typedef void (*ble_action_heartbeat_cb_t)(void);

/**
 * @brief Callback for action 1 - Execute ring animation
 * @param animation The animation to execute (EChevronAnimation)
 */
typedef void (*ble_action_animation_cb_t)(EChevronAnimation animation);

/**
 * @brief Callback for action 2 - Set lighted symbols
 * @param symbol_bits 48-bit pattern (6 bytes), 1 bit per symbol
 */
typedef void (*ble_action_symbols_cb_t)(const uint8_t symbol_bits[6]);

/**
 * @brief Callback for action 3 - Power off
 */
typedef void (*ble_action_poweroff_cb_t)(void);

/**
 * @brief Callback for action 4 - Light up single symbol
 * @param symbol_index The symbol index to light up (0-47)
 * @param level Light level PWM
 */
typedef void (*ble_action_light_symbol_cb_t)(uint8_t symbol_index, uint8_t level);

/**
 * @brief Callback for action 5 - Go to factory mode
 */
typedef void (*ble_action_goto_factory_cb_t)(void);

/**
 * @brief Structure containing all BLE action callbacks
 */
typedef struct {
    ble_action_heartbeat_cb_t heartbeat_cb;
    ble_action_animation_cb_t animation_cb;
    ble_action_symbols_cb_t symbols_cb;
    ble_action_poweroff_cb_t poweroff_cb;
    ble_action_light_symbol_cb_t light_symbol_cb;
    ble_action_goto_factory_cb_t goto_factory_cb;
} ble_action_callbacks_t;

/**
 * @brief Initialize and start the BLE server
 *
 * This function initializes the NimBLE stack and starts a BLE server
 * with one service and one characteristic.
 *
 * @param callbacks Pointer to structure containing action callbacks (can be NULL)
 * @return 0 on success, non-zero on failure
 */
int ble_server_init(const ble_action_callbacks_t *callbacks);

/**
 * @brief Stop the BLE server
 */
void ble_server_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SERVER_H
