#include "ble_server.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "BLE_SERVER";

// Define a custom 128-bit UUID for the service
// You can generate your own UUID using online tools
// Example: 12345678-1234-5678-1234-56789abcdef0
static const ble_uuid128_t service_uuid =
    BLE_UUID128_INIT(0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

// Define a custom 128-bit UUID for the characteristic
// Example: 12345678-1234-5678-1234-56789abcdef1
static const ble_uuid128_t char_uuid =
    BLE_UUID128_INIT(0xf1, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

// Storage for characteristic value
static uint8_t char_value[20] = {0};
static uint16_t char_value_handle;

// Action callbacks
static ble_action_callbacks_t g_action_callbacks = {0};

static void ble_advertise(void);

/**
 * @brief Callback for characteristic access
 */
static int ble_characteristic_access(uint16_t conn_handle, uint16_t attr_handle,
                                     struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc;

    ESP_LOGI(TAG, "test");

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        ESP_LOGI(TAG, "Characteristic read");
        rc = os_mbuf_append(ctxt->om, &char_value, sizeof(char_value));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        ESP_LOGI(TAG, "Characteristic write");
        uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
        if (om_len > sizeof(char_value)) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        if (om_len < 1) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        rc = ble_hs_mbuf_to_flat(ctxt->om, char_value, sizeof(char_value), NULL);
        if (rc != 0) {
            return BLE_ATT_ERR_UNLIKELY;
        }

        // Parse action type (first byte)
        uint8_t action_type = char_value[0];
        uint8_t payload_len = om_len - 1;
        uint8_t *payload = &char_value[1];

        ESP_LOGI(TAG, "Action type: %u, payload length: %u", action_type, payload_len);

        // Handle different action types
        switch ((ble_action_type_t)action_type) {
            case BLE_ACTION_HEARTBEAT:
                ESP_LOGI(TAG, "Action: Heartbeat");
                if (g_action_callbacks.heartbeat_cb) {
                    g_action_callbacks.heartbeat_cb();
                }
                break;

            case BLE_ACTION_ANIMATION:
                if (payload_len >= 1) {
                    EChevronAnimation animation = (EChevronAnimation)payload[0];
                    ESP_LOGI(TAG, "Action: Animation %u", (unsigned int)animation);
                    if (g_action_callbacks.animation_cb) {
                        g_action_callbacks.animation_cb(animation);
                    }
                } else {
                    ESP_LOGW(TAG, "Animation action requires at least 1 byte payload");
                    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
                }
                break;

            case BLE_ACTION_SET_SYMBOLS:
                if (payload_len >= 6) {
                    ESP_LOGI(TAG, "Action: Set symbols");
                    if (g_action_callbacks.symbols_cb) {
                        g_action_callbacks.symbols_cb(payload);
                    }
                } else {
                    ESP_LOGW(TAG, "Symbols action requires 6 bytes payload");
                    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
                }
                break;

            case BLE_ACTION_POWER_OFF:
                ESP_LOGI(TAG, "Action: Power off");
                if (g_action_callbacks.poweroff_cb) {
                    g_action_callbacks.poweroff_cb();
                }
                break;

            case BLE_ACTION_LIGHT_SYMBOL:
                if (payload_len >= 2) {
                    uint8_t symbol_index = payload[0];
                    uint8_t level_pwm = payload[1];
                    ESP_LOGI(TAG, "Action: Light symbol %u, level_pwm: %u", symbol_index, level_pwm);
                    if (g_action_callbacks.light_symbol_cb) {
                        g_action_callbacks.light_symbol_cb(symbol_index, level_pwm);
                    }
                } else {
                    ESP_LOGW(TAG, "Light symbol action requires at least 1 byte payload");
                    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
                }
                break;

            case BLE_ACTION_GOTO_FACTORY:
                ESP_LOGI(TAG, "Action: Goto factory");
                if (g_action_callbacks.goto_factory_cb) {
                    g_action_callbacks.goto_factory_cb();
                }
                break;

            default:
                ESP_LOGW(TAG, "Unknown action type: %u", action_type);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        return 0;

    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

/**
 * @brief GATT server service definition
 */
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        // Service definition
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                // Characteristic definition
                .uuid = &char_uuid.u,
                .access_cb = ble_characteristic_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = &char_value_handle,
            },
            {
                0, // No more characteristics in this service
            }
        },
    },
    {
        0, // No more services
    },
};

/**
 * @brief GAP event handler
 */
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) 
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "Connection %s; status=%d",
                 event->connect.status == 0 ? "established" : "failed",
                 event->connect.status);
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnect; reason=%d", event->disconnect.reason);
        // Start advertising again
        ble_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertising complete");
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
        ESP_LOGI(TAG, "Connection updated");
        break;

    default:
        break;
    }
    return 0;
}

/**
 * @brief Start advertising
 */
static void ble_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    // Set advertising data
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)"RingFW-BLE";
    fields.name_len = strlen("RingFW-BLE");
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error setting advertisement data; rc=%d", rc);
        return;
    }

    // Start advertising
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                          &adv_params, ble_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error starting advertisement; rc=%d", rc);
        return;
    }

    ESP_LOGI(TAG, "Advertising started");
}

/**
 * @brief Callback when the BLE host is synchronized
 */
static void ble_on_sync(void)
{
    ESP_LOGI(TAG, "BLE host synchronized");
    ble_advertise();
}

/**
 * @brief Callback when the BLE host resets
 */
static void ble_on_reset(int reason)
{
    ESP_LOGE(TAG, "BLE host reset; reason=%d", reason);
}

/**
 * @brief NimBLE host task
 */
static void ble_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

int ble_server_init(const ble_action_callbacks_t *callbacks)
{
    int rc;

    ESP_LOGI(TAG, "Initializing BLE server");

    // Store callbacks if provided
    if (callbacks) {
        g_action_callbacks = *callbacks;
    }

    // Initialize NimBLE
    ESP_ERROR_CHECK(nimble_port_init());

    // Initialize the NimBLE host configuration
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.reset_cb = ble_on_reset;

    // Register GATT services
    rc = ble_gatts_count_cfg(gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to count GATT services; rc=%d", rc);
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to add GATT services; rc=%d", rc);
        return rc;
    }

    // Set device name
    rc = ble_svc_gap_device_name_set("RingFW-BLE");
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to set device name; rc=%d", rc);
        return rc;
    }

    // Initialize GAP and GATT services
    ble_svc_gap_init();
    ble_svc_gatt_init();

    // Start the BLE host task
    nimble_port_freertos_init(ble_host_task);

    ESP_LOGI(TAG, "BLE server initialized successfully");
    return 0;
}

void ble_server_deinit(void)
{
    nimble_port_stop();
    nimble_port_deinit();
    ESP_LOGI(TAG, "BLE server deinitialized");
}
