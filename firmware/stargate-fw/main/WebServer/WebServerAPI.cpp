#include "WebServer.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_ota_ops.h"
#include "nvsjson.h"
#include "../Settings.hpp"
#include "cJSON.h"

#define TAG "WebAPI"

/*! @brief this variable is set by linker script, don't rename it. It contains app image informations. */
extern const esp_app_desc_t esp_app_desc;

esp_err_t WebServer::WebAPIGetHandler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    char* pExportJSON = NULL;

    if (strcmp(req->uri, APIURL_GETSTATUS_URI) == 0)
    {
    }
    else if (strcmp(req->uri, APIURL_GETSYSINFO_URI) == 0)
    {
        pExportJSON = GetSysInfo();
        if (pExportJSON == NULL || httpd_resp_send_chunk(req, pExportJSON, strlen(pExportJSON)) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to send data");
            goto ERROR;
        }
    }
    else if (strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI) == 0)
    {
        pExportJSON = Settings::getI().ExportJSON();
        if (pExportJSON == NULL || httpd_resp_send_chunk(req, pExportJSON, strlen(pExportJSON)) != ESP_OK)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to send data");
            goto ERROR;
        }
    }
    else if (strcmp(req->uri, APIURL_GET_FREERTOSDBGINFO_URI) == 0)
    {
        // According to the documentation, put a big buffer.
        char* pExportJSON = (char*)malloc(4096);
        if (pExportJSON == NULL)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Not enough memory");
            goto ERROR;
        }
        vTaskList(pExportJSON);
        if (httpd_resp_send_chunk(req, pExportJSON, strlen(pExportJSON)) != ESP_OK)
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

char* WebServer::GetSysInfo()
{
    cJSON* pRoot = NULL;
    {
        char buff[100];
        pRoot = cJSON_CreateObject();
        if (pRoot == NULL)
        {
            goto ERROR;
        }
        cJSON* pEntries = cJSON_AddArrayToObject(pRoot, "infos");

        // Firmware
        cJSON* pEntryJSON1 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON1, "name", cJSON_CreateString("Firmware"));
        cJSON_AddItemToObject(pEntryJSON1, "value", cJSON_CreateString(esp_app_desc.version));
        cJSON_AddItemToArray(pEntries, pEntryJSON1);

        // Compile Time
        cJSON* pEntryJSON2 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON2, "name", cJSON_CreateString("Compile Time"));
        sprintf(buff, "%s %s", /*0*/esp_app_desc.date, /*0*/esp_app_desc.time);
        cJSON_AddItemToObject(pEntryJSON2, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON2);

        // SHA256
        cJSON* pEntryJSON3 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON3, "name", cJSON_CreateString("SHA256"));
        char elfSHA256[sizeof(esp_app_desc.app_elf_sha256)*2 + 1] = {0,};
        ToHexString(elfSHA256, esp_app_desc.app_elf_sha256, sizeof(esp_app_desc.app_elf_sha256));
        cJSON_AddItemToObject(pEntryJSON3, "value", cJSON_CreateString(elfSHA256));
        cJSON_AddItemToArray(pEntries, pEntryJSON3);

        // IDF
        cJSON* pEntryJSON4 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON4, "name", cJSON_CreateString("IDF"));
        cJSON_AddItemToObject(pEntryJSON4, "value", cJSON_CreateString(esp_app_desc.idf_ver));
        cJSON_AddItemToArray(pEntries, pEntryJSON4);

        // WiFi-AP
        cJSON* pEntryJSON5 = cJSON_CreateObject();
        uint8_t u8Macs[6];
        cJSON_AddItemToObject(pEntryJSON5, "name", cJSON_CreateString("WiFi.AP"));
        esp_read_mac(u8Macs, ESP_MAC_WIFI_SOFTAP);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/u8Macs[0], /*1*/u8Macs[1], /*2*/u8Macs[2], /*3*/u8Macs[3], /*4*/u8Macs[4], /*5*/u8Macs[5]);
        cJSON_AddItemToObject(pEntryJSON5, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON5);

        // WiFi-STA
        cJSON* pEntryJSON6 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON6, "name", cJSON_CreateString("WiFi.STA"));
        esp_read_mac(u8Macs, ESP_MAC_WIFI_STA);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/u8Macs[0], /*1*/u8Macs[1], /*2*/u8Macs[2], /*3*/u8Macs[3], /*4*/u8Macs[4], /*5*/u8Macs[5]);
        cJSON_AddItemToObject(pEntryJSON6, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON6);

        // WiFi-BT
        cJSON* pEntryJSON7 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON7, "name", cJSON_CreateString("WiFi.BT"));
        esp_read_mac(u8Macs, ESP_MAC_BT);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/u8Macs[0], /*1*/u8Macs[1], /*2*/u8Macs[2], /*3*/u8Macs[3], /*4*/u8Macs[4], /*5*/u8Macs[5]);
        cJSON_AddItemToObject(pEntryJSON7, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON7);

        // Memory
        cJSON* pEntryJSON8 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON8, "name", cJSON_CreateString("Memory"));
        const int freeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        const int totalSize = heap_caps_get_total_size(MALLOC_CAP_8BIT);

        sprintf(buff, "%d / %d", /*0*/freeSize, /*1*/totalSize);
        cJSON_AddItemToObject(pEntryJSON8, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON8);

        // WiFi-station (IP address)
        /* cJSON* pEntryJSON9 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON9, "name", cJSON_CreateString("WiFi (STA)"));
        esp_netif_ip_info_t wifiIpSta = {0};
        MAIN_GetWiFiSTAIP(&wifiIpSta);
        sprintf(buff, IPSTR, IP2STR(&wifiIpSta.ip));
        cJSON_AddItemToObject(pEntryJSON9, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON9);

        esp_ip6_addr_t if_ip6[CONFIG_LWIP_IPV6_NUM_ADDRESSES] = {0};
        const int32_t s32IPv6Count = MAIN_GetWiFiSTAIPv6(if_ip6);
        for(int i = 0; i < HELPERMACRO_MIN(s32IPv6Count, 2); i++)
        {
            char ipv6String[45+1] = {0,};
            snprintf(ipv6String, sizeof(ipv6String)-1, IPV6STR, IPV62STR(if_ip6[i]));

            cJSON* pEntryJSONIPv6 = cJSON_CreateObject();
            cJSON_AddItemToObject(pEntryJSONIPv6, "name", cJSON_CreateString("WiFi (STA) IPv6"));
            cJSON_AddItemToObject(pEntryJSONIPv6, "value", cJSON_CreateString(ipv6String));
            cJSON_AddItemToArray(pEntries, pEntryJSONIPv6);
        }

        // WiFi-Soft AP (IP address)
        cJSON* pEntryJSON10 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON10, "name", cJSON_CreateString("WiFi (Soft-AP)"));
        esp_netif_ip_info_t wifiIpSoftAP = {0};
        MAIN_GetWiFiSoftAPIP(&wifiIpSoftAP);
        sprintf(buff, IPSTR, IP2STR(&wifiIpSoftAP.ip));
        cJSON_AddItemToObject(pEntryJSON10, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON10);
 */
        char* pStr =  cJSON_PrintUnformatted(pRoot);
        cJSON_Delete(pRoot);
        return pStr;
    }
    ERROR:
    cJSON_Delete(pRoot);
    return NULL;
}

void WebServer::ToHexString(char dstHexString[], const uint8_t* data, uint8_t len)
{
    for (uint32_t i = 0; i < len; i++)
        sprintf(dstHexString + (i * 2), "%02X", data[i]);
}