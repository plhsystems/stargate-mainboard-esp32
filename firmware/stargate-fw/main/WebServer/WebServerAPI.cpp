#include "WebServer.hpp"
#include "esp_log.h"
#include "nvsjson.h"
#include "../Settings.hpp"

#define TAG "WebAPI"

esp_err_t WebServer::WebAPIGetHandler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    char* pExportJSON = NULL;

    if (strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI) == 0)
    {
        pExportJSON = Settings::getI().ExportJSON();
        if (pExportJSON == NULL || httpd_resp_send_chunk(req, pExportJSON, strlen(pExportJSON)) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to send data");
            goto ERROR;
        }
    }
    else
    {
        ESP_LOGE(TAG, "api_get_handler, url: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Unknown request");
        goto ERROR;
    }
    goto END;
    ERROR:
    err = ESP_FAIL;
    END:
    if (pExportJSON != NULL)
        free(pExportJSON);
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_chunk(req, NULL, 0);
    return err;
}

esp_err_t WebServer::WebAPIPostHandler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    WebServer& ws = WebServer::getI();

    int n = 0;
    while(1)
    {
        int reqN = httpd_req_recv(req, (char*)ws.m_u8Buffers + n, HTTPSERVER_BUFFERSIZE - n - 1);
        if (reqN <= 0)
        {
            ESP_LOGI(TAG, "api_post_handler, test: %d, reqN: %d", (int)n, (int)reqN);
            break;
        }
        n += reqN;
    }
    ws.m_u8Buffers[n] = '\0';

    ESP_LOGI(TAG, "api_post_handler, url: %s", req->uri);
    if (strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI) == 0)
    {
        if (!Settings::getI().ImportJSON((const char*)ws.m_u8Buffers))
        {
            ESP_LOGE(TAG, "Unable to import JSON");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unknown request");
            goto ERROR;
        }
    }
    else
    {
        ESP_LOGE(TAG, "api_post_handler, url: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Unknown request");
        goto ERROR;
    }
    goto END;
    ERROR:
    err = ESP_FAIL;
    END:
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_chunk(req, NULL, 0);
    return err;
}
