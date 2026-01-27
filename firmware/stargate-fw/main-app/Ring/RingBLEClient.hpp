#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "SGUComm.hpp"
#include "esp_nimble_hci.h"
#include "host/ble_uuid.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

// Undefine NimBLE's min/max macros that conflict with C++ standard library
#undef min
#undef max

// Forward declarations for NimBLE
struct ble_gap_event;
struct ble_gatt_attr;
struct ble_gatt_error;
struct ble_gatt_chr;

class RingBLEClient
{
private:
    RingBLEClient();

    // Friend functions for BLE callbacks that need access to private members
    friend void OnBLESync();
    friend void OnBLEReset(int reason);
    friend int OnDiscoveryComplete(uint16_t conn_handle, const struct ble_gatt_error *error,
                                   const struct ble_gatt_chr *chr, void *arg);

public:
    // Singleton pattern
    RingBLEClient(RingBLEClient const&) = delete;
    void operator=(RingBLEClient const&) = delete;

    static RingBLEClient& getI()
    {
        static RingBLEClient instance;
        return instance;
    }

    /**
     * @brief Initialize the BLE client
     */
    void Init();

    /**
     * @brief Start the BLE client task (scans and connects)
     */
    void Start();

    /**
     * @brief Check if connected to the ring
     */
    bool GetIsConnected();

    /**
     * @brief Send heartbeat action (action 0)
     */
    void SendHeartbeat();

    /**
     * @brief Send animation action (action 1)
     * @param animation The animation to execute
     */
    void SendAnimation(SGUCommNS::EChevronAnimation animation);

    /**
     * @brief Send set symbols action (action 2)
     * @param symbol_bits 48-bit pattern (6 bytes), 1 bit per symbol
     */
    void SendSetSymbols(const uint8_t symbol_bits[6]);

    /**
     * @brief Send power off action (action 3)
     */
    void SendPowerOff();

    /**
     * @brief Send light up single symbol action (action 4)
     * @param symbol_index The symbol index to light up (0-47)
     */
    void SendLightUpSymbol(uint8_t symbol_index);

    /**
     * @brief Send goto factory action (action 5)
     */
    void SendGotoFactory();

    /**
     * @brief Send gate animation (alias for SendAnimation for compatibility)
     */
    void SendGateAnimation(SGUCommNS::EChevronAnimation animation) { SendAnimation(animation); }

    /**
     * @brief Task running function
     */
    static void TaskRunning(void* arg);

    /**
     * @brief GAP event handler
     */
    static int GapEventHandler(struct ble_gap_event *event, void *arg);

private:
    bool LockMutex() { return (pdTRUE == xSemaphoreTake(m_mutex_handle, (TickType_t)pdMS_TO_TICKS(100))); }
    void UnlockMutex() { xSemaphoreGive(m_mutex_handle); }

    /**
     * @brief Write data to the characteristic
     */
    bool WriteCharacteristic(const uint8_t* data, uint16_t len);

    /**
     * @brief Start scanning for the ring
     */
    void StartScan();

    /**
     * @brief Connect to the ring
     */
    void ConnectToRing();

private:
    // Task handle
    TaskHandle_t m_task_handle = nullptr;

    // BLE connection
    uint16_t m_conn_handle = 0;
    uint16_t m_char_value_handle = 0;
    bool m_is_connected = false;
    bool m_char_discovered = false;

    // Ring device address
    uint8_t m_ring_addr[6] = {0};
    uint8_t m_ring_addr_type = 0;
    bool m_ring_found = false;

    // Mutex
    StaticSemaphore_t m_mutex_buffer;
    SemaphoreHandle_t m_mutex_handle;

    // Connection parameters
    static constexpr uint32_t SCAN_INTERVAL_MS = 5000;
    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 1000;
};
