#include "WebServer.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_ota_ops.h"
#include "nvsjson.h"
#include "../Settings.hpp"
#include "WifiMgr.hpp"
#include "esp_netif_types.h"
#include "misc-macro.h"
#include "../Gate/BaseGate.hpp"
#include "../Gate/GateFactory.hpp"
#include "../Gate/GateControl.hpp"
#include "../Ring/RingComm.hpp"
#include "../Audio/SoundFX.hpp"
#include "../HttpClient.hpp"

#define TAG "WebAPI"

/*! @brief this variable is set by linker script, don't rename it. It contains app image informations. */
extern const esp_app_desc_t esp_app_desc;

esp_err_t WebServer::WebAPIGetHandler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    char* pExportJSON = NULL;
    bool freeMem = true;

    if (strcmp(req->uri, APIURL_GETSTATUS_URI) == 0) {
        pExportJSON = getI().GetStatus();
    }
    else if (strcmp(req->uri, APIURL_GETSYSINFO_URI) == 0) {
        pExportJSON = getI().GetSysInfo();
    }
    else if (strcmp(req->uri, APIURL_GETSOUNDLIST_URI) == 0) {
        pExportJSON = getI().GetAllSoundLists();
    }
    else if (strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI) == 0) {
        pExportJSON = Settings::getI().ExportJSON();
    }
    else if (strcmp(req->uri, APIURL_GETFANGATELIST_MILKYWAY_URI) == 0) {
        freeMem = false;
        pExportJSON = (char*)HttpClient::getI().GetFanGateListString();
        if (nullptr == pExportJSON) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Fan gate list isn't available yet");
            goto ERROR;
        }
    }
    else if (strcmp(req->uri, APIURL_GETFREERTOSDBGINFO_URI) == 0) {
        // According to the documentation, put a big buffer.
        pExportJSON = (char*)malloc(4096);
        vTaskList(pExportJSON);
    }
    else if (strcmp(req->uri, APIURL_GALAXY_GETINFO_MILKYWAY_URI) == 0) {
        pExportJSON = getI().GetGalaxyInfoJSON(GateGalaxy::MilkyWay);
    }
    else if (strcmp(req->uri, APIURL_GALAXY_GETINFO_PEGASUS_URI) == 0) {
        pExportJSON = getI().GetGalaxyInfoJSON(GateGalaxy::Pegasus);
    }
    else if (strcmp(req->uri, APIURL_GALAXY_GETINFO_UNIVERSE_URI) == 0) {
        pExportJSON = getI().GetGalaxyInfoJSON(GateGalaxy::Universe);
    }
    else {
        ESP_LOGE(TAG, "api_get_handler, url: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Unknown request");
        goto ERROR;
    }

    // Allocation error
    if (pExportJSON == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Not enough memory");
        goto ERROR;
    }

    // Chunk file transfer
    httpd_resp_set_type(req, "application/json");

    if (httpd_resp_send_chunk(req, pExportJSON, strlen(pExportJSON)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to send data");
        goto ERROR;
    }
    goto END;
    ERROR:
    err = ESP_FAIL;
    END:
    httpd_resp_set_hdr(req, "Connection", "close");
    if (freeMem && pExportJSON != NULL) {
        free(pExportJSON);
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return err;
}

esp_err_t WebServer::WebAPIPostHandler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    cJSON* pRoot = nullptr;
    WebServer& ws = WebServer::getI();

    // Get datas
    int n = 0;
    while(1) {
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
    if (strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI) == 0) {
        // ==============================================
        // Import the JSON setting file
        if (!Settings::getI().ImportJSON((const char*)ws.m_u8Buffers)) {
            ESP_LOGE(TAG, "Unable to import JSON");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unknown request");
            goto ERROR;
        }
    }
    else
    {
        pRoot = cJSON_Parse((const char*)ws.m_u8Buffers);

        // ==============================================
        // Gate control
        if (strcmp(req->uri, APIURL_POSTCONTROL_AUTOHOME_URI) == 0) {
            GateControl::getI().QueueAutoHome();
        }
        else if (strcmp(req->uri, APIURL_POSTCONTROL_AUTOCALIBRATE_URI) == 0) {
            GateControl::getI().QueueAutoCalibrate();
        }
        else if (strcmp(req->uri, APIURL_POSTCONTROL_DIALADDRESS_URI) == 0) {
            const cJSON* jItemAddrs = cJSON_GetObjectItem(pRoot, "addr");
            if (nullptr == jItemAddrs ||
                !cJSON_IsArray(jItemAddrs)) {
                goto ERROR;
            }
            // Check for symbols
            uint8_t u8Symbols[GateAddress::SYMBOL_COUNT];
            uint8_t u8SymbolCount = 0;
            if (cJSON_GetArraySize(jItemAddrs) > GateAddress::SYMBOL_COUNT) {
                goto ERROR;
            }
            const cJSON* cJSONItem = NULL;
            cJSON_ArrayForEach(cJSONItem, jItemAddrs)
            {
                if (!cJSON_IsNumber(cJSONItem)) {
                    goto ERROR;
                }
                u8Symbols[u8SymbolCount] = (uint8_t)cJSONItem->valueint;
                u8SymbolCount++;
            }
            GateAddress ga { u8Symbols, u8SymbolCount };
            GateControl::getI().QueueDialAddress(ga);
        }
        else if (strcmp(req->uri, APIURL_POSTCONTROL_ABORT_URI) == 0) {
            GateControl::getI().AbortAction();
        }
        // Test control
        else if (strcmp(req->uri, APIURL_POSTCONTROL_TESTRAMPLIGHT_URI) == 0) {
            const cJSON* jItemValue = cJSON_GetObjectItem(pRoot, "value");
            if (nullptr == jItemValue ||
                !cJSON_IsNumber(jItemValue) ||
                jItemValue->valuedouble < 0.0d || jItemValue->valuedouble > 1.0d) {
                goto ERROR;
            }
            ws.m_pSGHWHAL->SetRampLight(jItemValue->valuedouble);
        }
        else if (strcmp(req->uri, APIURL_POSTCONTROL_TESTSERVO_URI) == 0) {
            const cJSON* jItemValue = cJSON_GetObjectItem(pRoot, "value");
            if (nullptr == jItemValue ||
                !cJSON_IsNumber(jItemValue) ||
                jItemValue->valuedouble < 0.0d || jItemValue->valuedouble > 1.0d) {
                goto ERROR;
            }
            ws.m_pSGHWHAL->SetServo(jItemValue->valuedouble);
        }
        // Sounds
        else if (strcmp(req->uri, APIURL_PLAYSOUND_URI) == 0) {
            const cJSON* jItemAnim = cJSON_GetObjectItem(pRoot, "id");
            if (nullptr == jItemAnim ||
                !cJSON_IsNumber(jItemAnim)) {
                goto ERROR;
            }
            if (!SoundFX::getI().PlaySound((SoundFX::FileID)(jItemAnim->valueint), false)) {
                goto ERROR;
            }
        }
        // Wormhole manual wormhole mode
        else if (strcmp(req->uri, APIURL_POSTCONTROL_MANUALWORMHOLE_URI) == 0) {
            const cJSON* jItemAnim = cJSON_GetObjectItem(pRoot, "id");
            if (nullptr == jItemAnim ||
                !cJSON_IsNumber(jItemAnim)) {
                goto ERROR;
            }
            // TODO: Implement the wormhole.
            ESP_LOGI(TAG, "Implement wormhole, id: %d", jItemAnim->valueint);
            GateControl::getI().QueueManualWormhole( (Wormhole::EType)jItemAnim->valueint );
        }
        else if (strcmp(req->uri, APIURL_STOPSOUND_URI) == 0) {
            SoundFX::getI().StopSound();
        }
        // ==============================================
        // Ring control
        else if (strcmp(req->uri, APIURL_POSTRINGCONTROL_POWEROFF_URI) == 0) {
            RingComm::getI().SendPowerOff();
        }
        else if (strcmp(req->uri, APIURL_POSTRINGCONTROL_TESTANIMATE_URI) == 0) {
            const cJSON* jItemAnim = cJSON_GetObjectItem(pRoot, "id");
            if (nullptr == jItemAnim ||
                !cJSON_IsNumber(jItemAnim) ||
                jItemAnim->valueint < 0 || jItemAnim->valueint >= (int)SGUCommNS::EChevronAnimation::Count) {
                goto ERROR;
            }
            RingComm::getI().SendGateAnimation((SGUCommNS::EChevronAnimation)jItemAnim->valueint);
        }
        else if (strcmp(req->uri, APIURL_POSTRINGCONTROL_GOTOFACTORY_URI) == 0) {
            RingComm::getI().SendGotoFactory();
        }
        else {
            ESP_LOGE(TAG, "api_post_handler, url: %s", req->uri);
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Unknown request");
            goto ERROR;
        }
    }
    goto END;
    ERROR:
    err = ESP_FAIL;
    ESP_LOGE(TAG, "api_post_handler, url: %s, execution failed", req->uri);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unable to complete the request");
    END:
    if (nullptr != pRoot) {
        cJSON_Delete(pRoot);
    }

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_chunk(req, NULL, 0);
    return err;
}

char* WebServer::GetStatus()
{
    cJSON* pRoot = NULL;
    {
        pRoot = cJSON_CreateObject();
        if (pRoot == NULL) {
            goto ERROR;
        }

         cJSON* pStatusEntry = cJSON_CreateObject();

        GateControl::UIState sState;
        GateControl::getI().GetState(sState);

        cJSON_AddItemToObject(pStatusEntry, "text", cJSON_CreateString(sState.szStatusText));
        cJSON_AddItemToObject(pStatusEntry, "cancel_request", cJSON_CreateBool(sState.bIsCancelRequested));

        cJSON_AddItemToObject(pStatusEntry, "error_text", cJSON_CreateString(sState.szLastError));
        cJSON_AddItemToObject(pStatusEntry, "is_error", cJSON_CreateBool(sState.bHasLastError));

        cJSON* pRingEntry = cJSON_CreateObject();
        RingComm& refRingComm = RingComm::getI();
        cJSON_AddItemToObject(pRingEntry, "is_connected", cJSON_CreateBool(refRingComm.GetIsConnected()));
        cJSON_AddItemToObject(pStatusEntry, "ring", pRingEntry);

        time_t now = 0;
        struct tm timeinfo = { 0 };
        time(&now);
        localtime_r(&now, &timeinfo);

        cJSON* pTimeEntry = cJSON_CreateObject();
        cJSON_AddItemToObject(pTimeEntry, "h", cJSON_CreateNumber(timeinfo.tm_hour));
        cJSON_AddItemToObject(pTimeEntry, "m", cJSON_CreateNumber(timeinfo.tm_min));
        cJSON_AddItemToObject(pStatusEntry, "time", pTimeEntry);

        cJSON_AddItemToObject(pRoot, "status", pStatusEntry);

        char* pStr =  cJSON_PrintUnformatted(pRoot);
        cJSON_Delete(pRoot);
        return pStr;
    }
    ERROR:
    cJSON_Delete(pRoot);
    return NULL;
}

char* WebServer::GetSysInfo()
{
    cJSON* pRoot = NULL;
    {
        char buff[100];
        uint8_t u8Macs[6];
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

        // Bluetooth (BT)
        cJSON* pEntryJSON7 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON7, "name", cJSON_CreateString("Mac (BT)"));
        esp_read_mac(u8Macs, ESP_MAC_BT);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/u8Macs[0], /*1*/u8Macs[1], /*2*/u8Macs[2], /*3*/u8Macs[3], /*4*/u8Macs[4], /*5*/u8Macs[5]);
        cJSON_AddItemToObject(pEntryJSON7, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON7);

        // Station mode
        // WiFi-STA
        cJSON* pEntryJSON6 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON6, "name", cJSON_CreateString("Mac (STA)"));
        esp_read_mac(u8Macs, ESP_MAC_WIFI_STA);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/u8Macs[0], /*1*/u8Macs[1], /*2*/u8Macs[2], /*3*/u8Macs[3], /*4*/u8Macs[4], /*5*/u8Macs[5]);
        cJSON_AddItemToObject(pEntryJSON6, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON6);

        // WiFi-station (IP address)
        cJSON* pEntryJSON9 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON9, "name", cJSON_CreateString("WiFi (STA IPv4)"));
        esp_netif_ip_info_t wifiIpSta;
        memset(&wifiIpSta, 0, sizeof(wifiIpSta));
        WifiMgr::getI().GetWiFiSTAIP(wifiIpSta);
        sprintf(buff, IPSTR, IP2STR(&wifiIpSta.ip));
        cJSON_AddItemToObject(pEntryJSON9, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON9);

        esp_ip6_addr_t if_ip6[CONFIG_LWIP_IPV6_NUM_ADDRESSES];
        memset(&if_ip6[0], 0, sizeof(if_ip6));
        const int32_t s32IPv6Count = WifiMgr::getI().GetWiFiSTAIPv6(if_ip6);
        for(int i = 0; i < MISCMACRO_MIN(s32IPv6Count, 2); i++)
        {
            char ipv6String[45+1] = {0,};
            snprintf(ipv6String, sizeof(ipv6String)-1, IPV6STR, IPV62STR(if_ip6[i]));

            cJSON* pEntryJSONIPv6 = cJSON_CreateObject();
            cJSON_AddItemToObject(pEntryJSONIPv6, "name", cJSON_CreateString("WiFi (STA IPv6)"));
            cJSON_AddItemToObject(pEntryJSONIPv6, "value", cJSON_CreateString(ipv6String));
            cJSON_AddItemToArray(pEntries, pEntryJSONIPv6);
        }

        // Soft Access point
        // WiFi-AP
        cJSON* pEntryJSON5 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON5, "name", cJSON_CreateString("Mac (Soft-AP)"));
        esp_read_mac(u8Macs, ESP_MAC_WIFI_SOFTAP);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/u8Macs[0], /*1*/u8Macs[1], /*2*/u8Macs[2], /*3*/u8Macs[3], /*4*/u8Macs[4], /*5*/u8Macs[5]);
        cJSON_AddItemToObject(pEntryJSON5, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON5);

        // WiFi-Soft AP (IP address)
        cJSON* pEntryJSON10 = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryJSON10, "name", cJSON_CreateString("WiFi (Soft-AP)"));
        esp_netif_ip_info_t wifiIpSoftAP;
        memset(&wifiIpSoftAP, 0, sizeof(wifiIpSoftAP));
        WifiMgr::getI().GetWiFiSoftAPIP(wifiIpSoftAP);
        sprintf(buff, IPSTR, IP2STR(&wifiIpSoftAP.ip));
        cJSON_AddItemToObject(pEntryJSON10, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryJSON10);

        // Memory
        cJSON* pEntryMemoryJSON = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryMemoryJSON, "name", cJSON_CreateString("Memory"));
        const int freeSize = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        const int totalSize = heap_caps_get_total_size(MALLOC_CAP_8BIT);

        sprintf(buff, "%d / %d", /*0*/(totalSize - freeSize), /*1*/totalSize);
        cJSON_AddItemToObject(pEntryMemoryJSON, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(pEntries, pEntryMemoryJSON);

        // Current time
        cJSON* pEntryCurrTimeJSON = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryCurrTimeJSON, "name", cJSON_CreateString("Time"));
        cJSON_AddItemToObject(pEntryCurrTimeJSON, "value", cJSON_CreateString(esp_log_system_timestamp()));
        cJSON_AddItemToArray(pEntries, pEntryCurrTimeJSON);

        // Uptime (s)
        cJSON* pEntryUpTimeJSON = cJSON_CreateObject();
        cJSON_AddItemToObject(pEntryUpTimeJSON, "name", cJSON_CreateString("Uptime (s)"));
        cJSON_AddItemToObject(pEntryUpTimeJSON, "value", cJSON_CreateNumber(esp_log_timestamp()/1000));
        cJSON_AddItemToArray(pEntries, pEntryUpTimeJSON);

        char* pStr =  cJSON_PrintUnformatted(pRoot);
        cJSON_Delete(pRoot);
        return pStr;
    }
    ERROR:
    cJSON_Delete(pRoot);
    return NULL;
}

char* WebServer::GetAllSoundLists()
{
    cJSON* pRoot = NULL;
    {
        pRoot = cJSON_CreateObject();
        if (pRoot == NULL)
            goto ERROR;

        cJSON* pEntries = cJSON_AddArrayToObject(pRoot, "files");
        for(int32_t i = 0; i < SoundFX::getI().GetFileCount(); i++)
        {
            const SoundFX::SoundFile* pFile = SoundFX::getI().GetFile((SoundFX::FileID)i);

            cJSON* pNewFile = cJSON_CreateObject();
            cJSON_AddItemToObject(pNewFile, "id", cJSON_CreateNumber(i));
            cJSON_AddItemToObject(pNewFile, "name", cJSON_CreateString(pFile->szName));
            cJSON_AddItemToObject(pNewFile, "desc", cJSON_CreateString(pFile->szDesc));
            cJSON_AddItemToArray(pEntries, pNewFile);
        }
        char* pStr = cJSON_PrintUnformatted(pRoot);
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
