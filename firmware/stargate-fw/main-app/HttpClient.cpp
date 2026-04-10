#include "HttpClient.hpp"
#include <cstring>
#include "esp_tls.h"
#include "esp_tls_errors.h"
#include "esp_crt_bundle.h"
#include "FWConfig.hpp"
#include "misc-macro.h"
#include "WifiMgr.hpp"

#define TAG "HttpClient"

HttpClient::HttpClient()
    : m_fanGate(nullptr),
      m_last_update_ticks(0)
{
    m_fanGate_mutex = xSemaphoreCreateMutex();
}

void HttpClient::Init()
{

}

void HttpClient::Start()
{
    return; // Disable for now, as the API is not yet ready and we don't want to spam the server with failed requests.
    if (pdPASS != xTaskCreatePinnedToCore(TaskRunning, "HttpClient", FWCONFIG_HTTPCLIENT_STACKSIZE, (void*)this, FWCONFIG_HTTPCLIENT_PRIORITY_DEFAULT, &m_task_http_client_handle, FWCONFIG_HTTPCLIENT_COREID))
    {
        ESP_ERROR_CHECK(ESP_FAIL);
    }
}

std::shared_ptr<char[]> HttpClient::GetFanGateListString()
{
    std::shared_ptr<char[]> result;
    if (pdTRUE == xSemaphoreTake(m_fanGate_mutex, portMAX_DELAY))
    {
        result = m_fanGate;  // Safe copy
        xSemaphoreGive(m_fanGate_mutex);
    }
    return result;
}

void HttpClient::TaskRunning(void* arg)
{
    HttpClient* http_client = (HttpClient*)arg;

    int32_t fast_attempt = FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT;

    while(true)
    {
        esp_http_client_handle_t h = nullptr;
        bool error = false;
        std::shared_ptr<char[]> buffer;

        do {
            // Check if Wi-Fi is connected before attempting HTTP request
            if (WifiMgr::EState::Connected != WifiMgr::getI().GetWifiSTAState())
            {
                ESP_LOGW(TAG, "Wi-Fi not connected, skipping HTTP request");
                error = true;
                break;
            }

            const char* url = "https://api.thestargateproject.com/get_fan_gates.php";

            ESP_LOGI(TAG, "Opening url '%s'", url);

            const esp_http_client_config_t config =
            {
                .url = url,
                .auth_type = HTTP_AUTH_TYPE_NONE,
                .method = HTTP_METHOD_GET,
                .timeout_ms = 10000,

                .disable_auto_redirect = false,

                .transport_type = HTTP_TRANSPORT_OVER_SSL,
                // CRT bundle.
                .use_global_ca_store = true,
                .skip_cert_common_name_check = true,
                .crt_bundle_attach = esp_crt_bundle_attach,
            };

            h = esp_http_client_init(&config);
            if (nullptr == h)
            {
                ESP_LOGE(TAG, "esp_http_client_init failed (out of memory?)");
                error = true;
                break;
            }

            ESP_LOGD(TAG, "HTTP connection in progress ... url: %s", config.url);
            const esp_err_t err = esp_http_client_open(h, 0);
            if (ESP_OK != err)
            {
                ESP_LOGE(TAG, "Unable to open HTTP client connection, returned: %d, text: %s", err, esp_err_to_name(err));
                error = true;
                break;
            }
            ESP_LOGI(TAG, "HTTP connection succeeded! ");

            esp_http_client_fetch_headers(h);
            const int status_code = esp_http_client_get_status_code(h);
            if (200 != status_code)
            {
                ESP_LOGE(TAG, "HTTP server didn't return 200, it returned: %d", status_code);
                error = true;
                break;
            }

            // Read body in chunks into a static buffer to avoid heap fragmentation.
            // Content-Length may be -1 for chunked transfer-encoded responses.
            static constexpr int READ_CHUNK = 512;
            static constexpr int MAX_SIZE   = 32768;
            static char s_read_buf[MAX_SIZE + 1];
            int offset = 0;

            while(offset < MAX_SIZE)
            {
                const int n = esp_http_client_read(h, s_read_buf + offset, READ_CHUNK);
                if (0 > n)
                {
                    ESP_LOGE(TAG, "HTTP Read error, it returned: %d", n);
                    error = true;
                    break;
                }
                if (0 == n)
                {
                    break;  // EOF
                }
                offset += n;
            }

            if (error)
            {
                break;
            }

            s_read_buf[offset] = '\0';
            ESP_LOGI(TAG, "URL: %s, size: %d", url, offset);

            // Copy only the actual data into a right-sized heap allocation
            buffer = std::make_shared<char[]>(offset + 1);
            memcpy(buffer.get(), s_read_buf, offset + 1);

            if (pdTRUE == xSemaphoreTake(http_client->m_fanGate_mutex, portMAX_DELAY))
            {
                http_client->m_fanGate = buffer;
                http_client->m_last_update_ticks = xTaskGetTickCount();
                xSemaphoreGive(http_client->m_fanGate_mutex);
            }

        } while(false);

        // Cleanup
        if (nullptr != h)
        {
            esp_http_client_close(h);
            esp_http_client_cleanup(h);
        }

        // Fast retry (30s)
        if (error && fast_attempt-- >= 0)
        {
            ESP_LOGI(TAG, "Fast attempt in 30 s");
            vTaskDelay(pdMS_TO_TICKS(30*1000));
        }
        // Wait 10 minutes
        else
        {
            fast_attempt = FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT;
            ESP_LOGI(TAG, "Next refresh in 10 min");
            vTaskDelay(pdMS_TO_TICKS(10*60*1000));
        }
    }
    http_client->m_task_http_client_handle = nullptr;
    vTaskDelete(nullptr);
}