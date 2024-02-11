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
    int32_t s32FastAttempt = FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT;

    while(true)
    {
        bool error = false;
        {
            const char* szURL = "https://api.thestargateproject.com/get_fan_gates.php";

            ESP_LOGI(TAG, "Opening url '%s'", szURL);

            const esp_http_client_config_t config =
            {
                .url = szURL,
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

            auto sptrBuffer = make_shared<uint8_t[]>(len+1);
            uint8_t* u8Buffers = sptrBuffer.get();

            int offset = 0;
            while(offset < len)
            {
                int n = esp_http_client_read(h, (char*)u8Buffers + offset, len);
                if (n < 0) { // If we didn't receive correct byte count we call it an error
                    ESP_LOGE(TAG, "HTTP Read error, it returned: %d", statusCode);
                    goto ERROR;
                }
                offset += n;
            }
            u8Buffers[offset] = 0;
            ESP_LOGI(TAG, "URL: %s, size: %d / %d", szURL, offset, len);
            goto END;
        }
        ERROR:
        error = true;
        END:
        if (h != NULL) {
            esp_http_client_close(h);
            esp_http_client_cleanup(h);
        }

        // Fast retry (30s)
        if (error && s32FastAttempt-- >= 0) {
            ESP_LOGI(TAG, "Fast attempt in 30 s");
            vTaskDelay(pdMS_TO_TICKS(30*1000));
        }
        // Wait 10 minutes
        else {
            s32FastAttempt = FWCONFIG_HTTPCLIENT_FASTATTEMPT_COUNT;
            ESP_LOGI(TAG, "Next refresh in 10 min");
            vTaskDelay(pdMS_TO_TICKS(10*60*1000));
        }
    }
    getI().m_sTaskHTTPClientHandle = NULL;
    vTaskDelete(NULL);
}
