#include "HttpClient.hpp"
#include "esp_tls.h"
#include "esp_tls_errors.h"
#include "esp_crt_bundle.h"
#include "FWConfig.hpp"
#include "misc-macro.h"
#include <memory>

using namespace std;

#define TAG "HttpClient"

HttpClient::HttpClient()
{

}

void HttpClient::Init()
{

}

void HttpClient::Start()
{
	if (xTaskCreatePinnedToCore(TaskRunning, "HttpClient", FWCONFIG_HTTPCLIENT_STACKSIZE, (void*)this, FWCONFIG_HTTPCLIENT_PRIORITY_DEFAULT, &m_sTaskHTTPClientHandle, FWCONFIG_HTTPCLIENT_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
}

void HttpClient::TaskRunning(void* pArg)
{
    esp_http_client_handle_t h = NULL;

    for(int i = 0; i < 30; i++)
    {
        {
            const char* szURL = "https://api.thestargateproject.com/get_fan_gates.php";

            ESP_LOGI(TAG, "Testing URL #%d: '%s'", i, szURL);

            const esp_http_client_config_t config =
            {
                .url = szURL,
                .auth_type = HTTP_AUTH_TYPE_NONE,
                .method = HTTP_METHOD_GET,
                .timeout_ms = 60000,

                .disable_auto_redirect = false,

               // .event_handler = client_event_get_handler,

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
                goto ERROR;
            }
            ESP_LOGI(TAG, "HTTP connection succeeded! ");

            esp_http_client_fetch_headers(h);
            const int statusCode = esp_http_client_get_status_code(h);
            if (statusCode != 200)
            {
                ESP_LOGE(TAG, "HTTP server didn't return 200, it returned: %d", statusCode);
                goto ERROR;
            }

            const int len = esp_http_client_get_content_length(h);

            auto m_u8Buffers = make_shared<char[]>(len+1);

            int offset = 0;
            while(offset < len)
            {
                int n = esp_http_client_read(h, (char*)m_u8Buffers.get() + offset, len);
                if (n < 0) { // If we didn't receive correct byte count we call it an error
                    ESP_LOGE(TAG, "HTTP Read error, it returned: %d", statusCode);
                    goto ERROR;
                }
                offset += n;
            }
            m_u8Buffers.get()[offset] = 0;
            ESP_LOGI(TAG, "URL: %s, size: %d / %d", szURL, offset, len);
            goto END;
        }
        ERROR:
        END:
        if (h != NULL) {
            esp_http_client_close(h);
            esp_http_client_cleanup(h);
        }

        // Wait 60s
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
    getI().m_sTaskHTTPClientHandle = NULL;
    vTaskDelete(NULL);
}

esp_err_t HttpClient::client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
        case HTTP_EVENT_ON_DATA:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        }
        case HTTP_EVENT_ON_FINISH:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        }
        case HTTP_EVENT_DISCONNECTED:
        {
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            /* int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }*/
            break;
        }
        default:
        {
            break;
        }
    }
    return ESP_OK;
}

