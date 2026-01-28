#include "RingBLEClient.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "../FWConfig.hpp"

static const char *TAG = "RingBLEClient";

// UUIDs matching the ring-fw BLE server
// Service UUID: 12345678-1234-5678-1234-56789abcdef0
static const ble_uuid128_t SERVICE_UUID =
    BLE_UUID128_INIT(0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

// Define a custom 128-bit UUID for the characteristic
// Example: 12345678-1234-5678-1234-56789abcdef1
static const ble_uuid128_t CHAR_UUID =
    BLE_UUID128_INIT(0xf1, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

// Storage for characteristic value
// Action types enum
typedef enum {
    BLE_ACTION_HEARTBEAT = 0,
    BLE_ACTION_ANIMATION = 1,
    BLE_ACTION_SET_SYMBOLS = 2,
    BLE_ACTION_POWER_OFF = 3,
    BLE_ACTION_LIGHT_SYMBOL = 4,
    BLE_ACTION_GOTO_FACTORY = 5,
} ble_action_type_t;

RingBLEClient::RingBLEClient()
{
}

void RingBLEClient::Init()
{
    m_mutex_handle = xSemaphoreCreateMutexStatic(&m_mutex_buffer);
    ESP_LOGD(TAG, "RingBLEClient initialized");
}

void RingBLEClient::Start()
{
    LockMutex();
    ESP_LOGI(TAG, "Starting Ring BLE client");

    if (xTaskCreatePinnedToCore(TaskRunning, "RingBLE", 4096, (void*)this,
                                 FWCONFIG_RINGCOMM_PRIORITY_DEFAULT,
                                 &m_task_handle,
                                 FWCONFIG_RINGCOMM_COREID) != pdPASS)
    {
        ESP_ERROR_CHECK(ESP_FAIL);
    }
    UnlockMutex();
}

bool RingBLEClient::GetIsConnected()
{
    return m_is_connected && m_char_discovered;
}

void RingBLEClient::SendHeartbeat()
{
    if (!GetIsConnected()) {
        ESP_LOGW(TAG, "Not connected, cannot send heartbeat");
        return;
    }

    LockMutex();
    uint8_t data[1] = { BLE_ACTION_HEARTBEAT };
    WriteCharacteristic(data, 1);
    UnlockMutex();
}

void RingBLEClient::SendAnimation(SGUCommNS::EChevronAnimation animation)
{
    if (!GetIsConnected()) {
        ESP_LOGW(TAG, "Not connected, cannot send animation");
        return;
    }

    LockMutex();
    uint8_t data[2] = { BLE_ACTION_ANIMATION, (uint8_t)animation };
    WriteCharacteristic(data, 2);
    UnlockMutex();
}

void RingBLEClient::SendSetSymbols(const uint8_t symbol_bits[6])
{
    if (!GetIsConnected()) {
        ESP_LOGW(TAG, "Not connected, cannot send symbols");
        return;
    }

    LockMutex();
    uint8_t data[7];
    data[0] = BLE_ACTION_SET_SYMBOLS;
    memcpy(&data[1], symbol_bits, 6);
    WriteCharacteristic(data, 7);
    UnlockMutex();
}

void RingBLEClient::SendPowerOff()
{
    if (!GetIsConnected()) {
        ESP_LOGW(TAG, "Not connected, cannot send power off");
        return;
    }

    LockMutex();
    uint8_t data[1] = { BLE_ACTION_POWER_OFF };
    WriteCharacteristic(data, 1);
    UnlockMutex();
}

void RingBLEClient::SendLightUpSymbol(uint8_t symbol_index)
{
    if (!GetIsConnected()) {
        ESP_LOGW(TAG, "Not connected, cannot send light symbol");
        return;
    }

    LockMutex();
    const uint8_t data[3] = { BLE_ACTION_LIGHT_SYMBOL, symbol_index, 25 };
    WriteCharacteristic(data, sizeof(data));
    UnlockMutex();
}

void RingBLEClient::SendGotoFactory()
{
    if (!GetIsConnected()) {
        ESP_LOGW(TAG, "Not connected, cannot send goto factory");
        return;
    }

    LockMutex();
    uint8_t data[1] = { BLE_ACTION_GOTO_FACTORY };
    WriteCharacteristic(data, 1);
    UnlockMutex();
}

bool RingBLEClient::WriteCharacteristic(const uint8_t* data, uint16_t len)
{
    if (!m_char_discovered) {
        ESP_LOGE(TAG, "Characteristic not discovered, cannot write");
        return false;
    }

    if (!m_is_connected) {
        ESP_LOGE(TAG, "Not connected, cannot write");
        return false;
    }

    // Log the data being sent for debugging
    ESP_LOGD(TAG, "Writing %d bytes to char handle=%d, conn_handle=%d",
             len, m_char_value_handle, m_conn_handle);
    //ESP_LOG_BUFFER_HEX_LEVEL(TAG, data, len, ESP_LOG_INFO);

    int rc = ble_gattc_write_flat(m_conn_handle, m_char_value_handle,
                                   data, len, NULL, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to write characteristic: rc=%d (0x%x)", rc, rc);
        return false;
    }

    ESP_LOGD(TAG, "Write initiated successfully");
    return true;
}

int OnDiscoveryComplete(uint16_t conn_handle,
                        const struct ble_gatt_error *error,
                        const struct ble_gatt_chr *chr,
                        void *arg)
{
    RingBLEClient* client = (RingBLEClient*)arg;

    if (error->status != 0) {
        ESP_LOGE(TAG, "Characteristic discovery failed: status=%d", error->status);
        return 0;
    }

    if (chr == NULL) {
        ESP_LOGI(TAG, "Characteristic discovery complete");
        return 0;
    }

    // Log discovered characteristic
    ESP_LOGI(TAG, "Discovered characteristic: val_handle=%d, def_handle=%d, properties=0x%02x",
             chr->val_handle, chr->def_handle, chr->properties);

    // Store the characteristic handle (we want the WRITE characteristic)
    // The ring-fw has only one characteristic with WRITE property
    if (chr->properties & BLE_GATT_CHR_F_WRITE) {
        ESP_LOGI(TAG, "Found WRITE characteristic, storing handle=%d", chr->val_handle);
        // TODO: Fix this for real, ensure to use the right characteristic UUID instead
        if (RingBLEClient::getI().m_char_discovered) {
            ESP_LOGI(TAG, "Found WRITE characteristic (already found), storing handle=%d", chr->val_handle);
        }
        else {
            RingBLEClient::getI().m_char_value_handle = chr->val_handle;
            RingBLEClient::getI().m_char_discovered = true;

            // Connection animation
            client->SendAnimation(SGUCommNS::EChevronAnimation::Chevron_FadeIn);
        }
    }

    return 0;
}

int RingBLEClient::GapEventHandler(struct ble_gap_event *event, void *arg)
{
    RingBLEClient* client = (RingBLEClient*)arg;

    switch (event->type)
    {
        case BLE_GAP_EVENT_DISC:
        {
            // Device discovered during scan
            struct ble_hs_adv_fields fields;
            int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);

            if (rc == 0 && fields.name != NULL)
            {
                // Check if this is the RingFW-BLE device
                if (fields.name_len == 10 && memcmp(fields.name, "RingFW-BLE", 10) == 0) {
                    ESP_LOGI(TAG, "Found Ring device!");

                    client->m_ring_found = true;
                    memcpy(client->m_ring_addr, event->disc.addr.val, 6);
                    client->m_ring_addr_type = event->disc.addr.type;

                    // Stop scanning
                    ble_gap_disc_cancel();
                }
            }
            break;
        }

        case BLE_GAP_EVENT_CONNECT:
        {
            ESP_LOGI(TAG, "Connection %s; status=%d",
                     event->connect.status == 0 ? "established" : "failed",
                     event->connect.status);

            if (event->connect.status == 0) {
                client->m_conn_handle = event->connect.conn_handle;
                client->m_is_connected = true;

                // Discover the characteristic
                ble_uuid_any_t uuid;
                memcpy(&uuid.u128, &CHAR_UUID, sizeof(ble_uuid128_t));
                uuid.u.type = BLE_UUID_TYPE_128;

                int rc = ble_gattc_disc_all_chrs(event->connect.conn_handle,
                                                  1, 0xffff,
                                                  OnDiscoveryComplete, client);
                if (rc != 0) {
                    ESP_LOGE(TAG, "Failed to discover characteristics: rc=%d", rc);
                }
            } else {
                // Connection failed, retry
                client->m_is_connected = false;
                client->m_ring_found = false;
            }
            break;
        }

        case BLE_GAP_EVENT_DISCONNECT:
        {
            ESP_LOGI(TAG, "Disconnected; reason=%d", event->disconnect.reason);
            client->m_is_connected = false;
            client->m_char_discovered = false;
            client->m_ring_found = false;
            break;
        }

        case BLE_GAP_EVENT_DISC_COMPLETE:
        {
            ESP_LOGI(TAG, "Scan complete");
            break;
        }

        default:
            break;
    }

    return 0;
}

void RingBLEClient::StartScan()
{
    struct ble_gap_disc_params disc_params;
    memset(&disc_params, 0, sizeof(disc_params));

    disc_params.filter_duplicates = 1;
    disc_params.passive = 0;
    disc_params.itvl = 0;
    disc_params.window = 0;
    disc_params.filter_policy = 0;
    disc_params.limited = 0;

    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params,
                          GapEventHandler, this);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to start scan; rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "Scanning for Ring device...");
    }
}

void RingBLEClient::ConnectToRing()
{
    if (!m_ring_found) {
        return;
    }

    struct ble_gap_conn_params conn_params;
    memset(&conn_params, 0, sizeof(conn_params));
    conn_params.scan_itvl = 0x0010;
    conn_params.scan_window = 0x0010;
    conn_params.itvl_min = BLE_GAP_INITIAL_CONN_ITVL_MIN;
    conn_params.itvl_max = BLE_GAP_INITIAL_CONN_ITVL_MAX;
    conn_params.latency = 0;
    conn_params.supervision_timeout = 0x0100;
    conn_params.min_ce_len = 0x0000;
    conn_params.max_ce_len = 0x0000;

    ble_addr_t addr;
    addr.type = m_ring_addr_type;
    memcpy(addr.val, m_ring_addr, 6);

    ESP_LOGI(TAG, "Connecting to Ring...");
    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &addr, 30000, &conn_params,
                             GapEventHandler, this);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to connect; rc=%d", rc);
        m_ring_found = false;
    }
}

void OnBLESync(void)
{
    ESP_LOGI(TAG, "BLE host synchronized");
    // Start scanning
    RingBLEClient::getI().StartScan();
}

void OnBLEReset(int reason)
{
    ESP_LOGE(TAG, "BLE host reset; reason=%d", reason);
}

static void BLEHostTask(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void RingBLEClient::TaskRunning(void* arg)
{
    RingBLEClient* client = (RingBLEClient*)arg;

    // Initialize NimBLE
    ESP_LOGI(TAG, "Initializing NimBLE");
    ESP_ERROR_CHECK(nimble_port_init());

    // Initialize the NimBLE host configuration
    ble_hs_cfg.sync_cb = OnBLESync;
    ble_hs_cfg.reset_cb = OnBLEReset;

    // Start the BLE host task
    nimble_port_freertos_init(BLEHostTask);

    TickType_t last_scan_attempt = 0;
    TickType_t last_heartbeat = 0;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));

        // If not connected, try to scan/connect periodically
        if (!client->m_is_connected)
        {
            if (pdTICKS_TO_MS(xTaskGetTickCount() - last_scan_attempt) > SCAN_INTERVAL_MS)
            {
                last_scan_attempt = xTaskGetTickCount();

                if (!client->m_ring_found) {
                    ESP_LOGI(TAG, "Starting scan...");
                    client->StartScan();
                } else {
                    // Try to connect
                    client->ConnectToRing();
                }
            }
        }
        else
        {
            // Send periodic heartbeat when connected
            if (pdTICKS_TO_MS(xTaskGetTickCount() - last_heartbeat) > HEARTBEAT_INTERVAL_MS)
            {
                last_heartbeat = xTaskGetTickCount();
                client->SendHeartbeat();
            }
        }
    }
}
