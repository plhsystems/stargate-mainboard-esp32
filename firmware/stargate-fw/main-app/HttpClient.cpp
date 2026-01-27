#include "HttpClient.hpp"
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

}

void HttpClient::Init()
{

}

void HttpClient::Start()
{
	if (xTaskCreatePinnedToCore(TaskRunning, "HttpClient", FWCONFIG_HTTPCLIENT_STACKSIZE, (void*)this, FWCONFIG_HTTPCLIENT_PRIORITY_DEFAULT, &m_task_http_client_handle, FWCONFIG_HTTPCLIENT_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
}

const char* HttpClient::GetFanGateListString()
{
    return static_cast<const char*>(m_fanGate.get());
}

void HttpClient::TaskRunning(void* arg)
{
    HttpClient* http_client = (HttpClient*)arg;

    int32_t fast_attempt = FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT;

    while(true)
    {
        esp_http_client_handle_t h = NULL;
        bool error = false;
        std::shared_ptr<char[]> sptr_buffer;

        do {
            // Check if Wi-Fi is connected before attempting HTTP request
            if (WifiMgr::getI().GetWifiSTAState() != WifiMgr::EState::Connected)
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

            ESP_LOGD(TAG, "HTTP connection in progress ... url: %s", config.url);
            esp_err_t err;
            if (ESP_OK != (err = esp_http_client_open(h, 0)))
            {
                ESP_LOGE(TAG, "Unable to open HTTP client connection, returned: %d, text: %s", err, esp_err_to_name(err));
                error = true;
                break;
            }
            ESP_LOGI(TAG, "HTTP connection succeeded! ");

            esp_http_client_fetch_headers(h);
            const int statusCode = esp_http_client_get_status_code(h);
            if (statusCode != 200)
            {
                ESP_LOGE(TAG, "HTTP server didn't return 200, it returned: %d", statusCode);
                error = true;
                break;
            }

            const int len = esp_http_client_get_content_length(h);

            sptr_buffer = std::make_shared<char[]>(len+1);
            char* buffers = sptr_buffer.get();

            int offset = 0;
            while(offset < len)
            {
                int n = esp_http_client_read(h, (char*)buffers + offset, len);
                if (n < 0) {
                    ESP_LOGE(TAG, "HTTP Read error, it returned: %d", n);
                    error = true;
                    break;
                }
                offset += n;
            }

            if (error) {
                break;
            }

            buffers[offset] = 0;
            ESP_LOGI(TAG, "URL: %s, size: %d / %d", url, offset, len);

            // Keep the data ...
            http_client->m_fanGate = sptr_buffer;
            http_client->m_last_update_ticks = xTaskGetTickCount();

        } while(false);

        // Cleanup
        if (h != NULL) {
            esp_http_client_close(h);
            esp_http_client_cleanup(h);
        }

        // Fast retry (30s)
        if (error && fast_attempt-- >= 0) {
            ESP_LOGI(TAG, "Fast attempt in 30 s");
            vTaskDelay(pdMS_TO_TICKS(30*1000));
        }
        // Wait 10 minutes
        else {
            fast_attempt = FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT;
            ESP_LOGI(TAG, "Next refresh in 10 min");
            vTaskDelay(pdMS_TO_TICKS(10*60*1000));
        }
    }
    http_client->m_task_http_client_handle = NULL;
    vTaskDelete(NULL);
}