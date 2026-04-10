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
#include "../Ring/RingBLEClient.hpp"
#include "../Audio/SoundFX.hpp"
#include "../HttpClient.hpp"

#define TAG "WebAPI"

/*! @brief this variable is set by linker script, don't rename it. It contains app image informations. */
extern const esp_app_desc_t esp_app_desc;

esp_err_t WebServer::WebAPIGetHandler(httpd_req_t* req)
{
    esp_err_t err = ESP_OK;
    char* export_json = nullptr;
    bool free_mem = true;

    if (0 == strcmp(req->uri, APIURL_GETSTATUS_URI))
    {
        export_json = getI().GetStatus();
    }
    else if (0 == strcmp(req->uri, APIURL_GETSYSINFO_URI))
    {
        export_json = getI().GetSysInfo();
    }
    else if (0 == strcmp(req->uri, APIURL_GETSOUNDLIST_URI))
    {
        export_json = getI().GetAllSoundLists();
    }
    else if (0 == strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI))
    {
        export_json = Settings::getI().ExportJSON();
    }
    else if (0 == strcmp(req->uri, APIURL_GETFANGATELIST_MILKYWAY_URI))
    {
        std::shared_ptr<char[]> fangate_ptr = HttpClient::getI().GetFanGateListString();
        export_json = strdup(nullptr != fangate_ptr ? fangate_ptr.get() : "[]");
    }
    else if (0 == strcmp(req->uri, APIURL_GETFREERTOSDBGINFO_URI))
    {
        // According to the documentation, put a big buffer.
        export_json = (char*)malloc(4096);
        vTaskList(export_json);
    }
    else if (0 == strcmp(req->uri, APIURL_GALAXY_GETINFO_MILKYWAY_URI))
    {
        export_json = getI().GetGalaxyInfoJSON(GateGalaxy::MilkyWay);
    }
    else if (0 == strcmp(req->uri, APIURL_GALAXY_GETINFO_PEGASUS_URI))
    {
        export_json = getI().GetGalaxyInfoJSON(GateGalaxy::Pegasus);
    }
    else if (0 == strcmp(req->uri, APIURL_GALAXY_GETINFO_UNIVERSE_URI))
    {
        export_json = getI().GetGalaxyInfoJSON(GateGalaxy::Universe);
    }
    else
    {
        ESP_LOGE(TAG, "api_get_handler, url: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Unknown request");
        goto ERROR;
    }

    // Allocation error
    if (nullptr == export_json)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Not enough memory");
        goto ERROR;
    }

    // Chunk file transfer
    httpd_resp_set_type(req, "application/json");

    if (ESP_OK != httpd_resp_send_chunk(req, export_json, strlen(export_json)))
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to send data");
        goto ERROR;
    }
    goto END;
    ERROR:
    err = ESP_FAIL;
    END:
    httpd_resp_set_hdr(req, "Connection", "close");
    if (free_mem && nullptr != export_json)
    {
        free(export_json);
    }
    httpd_resp_send_chunk(req, nullptr, 0);
    return err;
}

esp_err_t WebServer::WebAPIPostHandler(httpd_req_t* req)
{
    esp_err_t err = ESP_OK;
    cJSON* root = nullptr;
    WebServer& ws = WebServer::getI();

    // Get datas
    int n = 0;
    while(1)
    {
        const int req_n = httpd_req_recv(req, (char*)ws.m_buffers + n, HTTPSERVER_BUFFERSIZE - n - 1);
        if (0 >= req_n)
        {
            ESP_LOGI(TAG, "api_post_handler, test: %d, reqN: %d", (int)n, (int)req_n);
            break;
        }
        n += req_n;
    }
    ws.m_buffers[n] = '\0';

    ESP_LOGI(TAG, "api_post_handler, url: %s", req->uri);
    if (0 == strcmp(req->uri, APIURL_GETPOST_SETTINGSJSON_URI))
    {
        // ==============================================
        // Import the JSON setting file
        if (!Settings::getI().ImportJSON((const char*)ws.m_buffers))
        {
            ESP_LOGE(TAG, "Unable to import JSON");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unknown request");
            goto ERROR;
        }
    }
    else
    {
        root = cJSON_Parse((const char*)ws.m_buffers);

        // ==============================================
        // Gate control
        if (0 == strcmp(req->uri, APIURL_POSTCONTROL_AUTOHOME_URI))
        {
            GateControl::getI().QueueAutoHome();
        }
        else if (0 == strcmp(req->uri, APIURL_POSTCONTROL_AUTOCALIBRATE_URI))
        {
            GateControl::getI().QueueAutoCalibrate();
        }
        else if (0 == strcmp(req->uri, APIURL_POSTCONTROL_DIALADDRESS_URI))
        {
            const cJSON* item_addrs = cJSON_GetObjectItem(root, "addr");
            if (nullptr == item_addrs ||
                !cJSON_IsArray(item_addrs))
            {
                goto ERROR;
            }
            // Check for symbols
            uint8_t symbols[GateAddress::SYMBOL_COUNT];
            uint8_t symbol_count = 0;
            if (GateAddress::SYMBOL_COUNT < cJSON_GetArraySize(item_addrs))
            {
                goto ERROR;
            }
            const cJSON* json_item = nullptr;
            cJSON_ArrayForEach(json_item, item_addrs)
            {
                if (!cJSON_IsNumber(json_item))
                {
                    goto ERROR;
                }
                symbols[symbol_count] = (uint8_t)json_item->valueint;
                symbol_count++;
            }
            GateAddress ga { symbols, symbol_count };
            GateControl::getI().QueueDialAddress(ga);
        }
        else if (0 == strcmp(req->uri, APIURL_POSTCONTROL_ABORT_URI))
        {
            GateControl::getI().AbortAction();
        }
        // Test control
        else if (0 == strcmp(req->uri, APIURL_POSTCONTROL_TESTRAMPLIGHT_URI))
        {
            const cJSON* item_value = cJSON_GetObjectItem(root, "value");
            if (nullptr == item_value ||
                !cJSON_IsNumber(item_value) ||
                0.0d > item_value->valuedouble || 1.0d < item_value->valuedouble)
            {
                goto ERROR;
            }
            ws.m_sghw_hal->SetRampLight(item_value->valuedouble);
        }
        else if (0 == strcmp(req->uri, APIURL_POSTCONTROL_TESTSERVO_URI))
        {
            const cJSON* item_value = cJSON_GetObjectItem(root, "value");
            if (nullptr == item_value ||
                !cJSON_IsNumber(item_value) ||
                0.0d > item_value->valuedouble || 1.0d < item_value->valuedouble)
            {
                goto ERROR;
            }
            ws.m_sghw_hal->SetServo(item_value->valuedouble);
        }
        // Sounds
        else if (0 == strcmp(req->uri, APIURL_PLAYSOUND_URI))
        {
            const cJSON* item_anim = cJSON_GetObjectItem(root, "id");
            if (nullptr == item_anim ||
                !cJSON_IsNumber(item_anim))
            {
                goto ERROR;
            }
            if (SGResult::OK != SoundFX::getI().PlaySound((SoundFX::FileID)(item_anim->valueint), false))
            {
                goto ERROR;
            }
        }
        // Wormhole manual wormhole mode
        else if (0 == strcmp(req->uri, APIURL_POSTCONTROL_MANUALWORMHOLE_URI))
        {
            const cJSON* item_anim = cJSON_GetObjectItem(root, "id");
            if (nullptr == item_anim ||
                !cJSON_IsNumber(item_anim))
            {
                goto ERROR;
            }
            // TODO: Implement the wormhole.
            ESP_LOGI(TAG, "Implement wormhole, id: %d", item_anim->valueint);
            GateControl::getI().QueueManualWormhole( (Wormhole::EType)item_anim->valueint );
        }
        else if (0 == strcmp(req->uri, APIURL_STOPSOUND_URI))
        {
            SoundFX::getI().StopSound();
        }
        // ==============================================
        // Ring control
        else if (0 == strcmp(req->uri, APIURL_POSTRINGCONTROL_POWEROFF_URI))
        {
            RingBLEClient::getI().SendPowerOff();
        }
        else if (0 == strcmp(req->uri, APIURL_POSTRINGCONTROL_TESTANIMATE_URI))
        {
            const cJSON* item_anim = cJSON_GetObjectItem(root, "id");
            if (nullptr == item_anim ||
                !cJSON_IsNumber(item_anim) ||
                0 > item_anim->valueint || (int)SGUCommNS::EChevronAnimation::Count <= item_anim->valueint)
            {
                goto ERROR;
            }
            RingBLEClient::getI().SendGateAnimation((SGUCommNS::EChevronAnimation)item_anim->valueint);
        }
        else if (0 == strcmp(req->uri, APIURL_POSTRINGCONTROL_GOTOFACTORY_URI))
        {
            RingBLEClient::getI().SendGotoFactory();
        }
        else
        {
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
    if (nullptr != root)
    {
        cJSON_Delete(root);
    }

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_chunk(req, nullptr, 0);
    return err;
}

char* WebServer::GetStatus()
{
    cJSON* root = nullptr;
    {
        root = cJSON_CreateObject();
        if (nullptr == root)
        {
            goto ERROR;
        }

        cJSON* status_entry = cJSON_CreateObject();

        GateControl::UIState state;
        GateControl::getI().GetState(state);

        cJSON_AddItemToObject(status_entry, "text", cJSON_CreateString(state.status_text));
        cJSON_AddItemToObject(status_entry, "cancel_request", cJSON_CreateBool(state.is_cancel_requested));

        cJSON_AddItemToObject(status_entry, "error_text", cJSON_CreateString(state.last_error));
        cJSON_AddItemToObject(status_entry, "is_error", cJSON_CreateBool(state.has_last_error));

        cJSON* ring_entry = cJSON_CreateObject();
        RingBLEClient& ring_comm = RingBLEClient::getI();
        cJSON_AddItemToObject(ring_entry, "is_connected", cJSON_CreateBool(ring_comm.GetIsConnected()));
        cJSON_AddItemToObject(status_entry, "ring", ring_entry);

        time_t now = 0;
        struct tm timeinfo = { 0 };
        time(&now);
        localtime_r(&now, &timeinfo);

        cJSON* time_entry = cJSON_CreateObject();
        cJSON_AddItemToObject(time_entry, "h", cJSON_CreateNumber(timeinfo.tm_hour));
        cJSON_AddItemToObject(time_entry, "m", cJSON_CreateNumber(timeinfo.tm_min));
        cJSON_AddItemToObject(status_entry, "time", time_entry);

        cJSON_AddItemToObject(root, "status", status_entry);

        char* str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        return str;
    }
    ERROR:
    cJSON_Delete(root);
    return nullptr;
}

char* WebServer::GetSysInfo()
{
    cJSON* root = nullptr;
    {
        char buff[100];
        uint8_t macs[6];
        root = cJSON_CreateObject();
        if (nullptr == root)
        {
            goto ERROR;
        }
        cJSON* entries = cJSON_AddArrayToObject(root, "infos");

        // Firmware
        cJSON* entryJSON1 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON1, "name", cJSON_CreateString("Firmware"));
        cJSON_AddItemToObject(entryJSON1, "value", cJSON_CreateString(esp_app_desc.version));
        cJSON_AddItemToArray(entries, entryJSON1);

        // Compile Time
        cJSON* entryJSON2 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON2, "name", cJSON_CreateString("Compile Time"));
        sprintf(buff, "%s %s", /*0*/esp_app_desc.date, /*0*/esp_app_desc.time);
        cJSON_AddItemToObject(entryJSON2, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entryJSON2);

        // SHA256
        cJSON* entryJSON3 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON3, "name", cJSON_CreateString("SHA256"));
        char elf_sha256[sizeof(esp_app_desc.app_elf_sha256)*2 + 1] = {0,};
        ToHexString(elf_sha256, esp_app_desc.app_elf_sha256, sizeof(esp_app_desc.app_elf_sha256));
        cJSON_AddItemToObject(entryJSON3, "value", cJSON_CreateString(elf_sha256));
        cJSON_AddItemToArray(entries, entryJSON3);

        // IDF
        cJSON* entryJSON4 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON4, "name", cJSON_CreateString("IDF"));
        cJSON_AddItemToObject(entryJSON4, "value", cJSON_CreateString(esp_app_desc.idf_ver));
        cJSON_AddItemToArray(entries, entryJSON4);

        // Bluetooth (BT)
        cJSON* entryJSON7 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON7, "name", cJSON_CreateString("Mac (BT)"));
        esp_read_mac(macs, ESP_MAC_BT);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/macs[0], /*1*/macs[1], /*2*/macs[2], /*3*/macs[3], /*4*/macs[4], /*5*/macs[5]);
        cJSON_AddItemToObject(entryJSON7, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entryJSON7);

        // Station mode
        // WiFi-STA
        cJSON* entryJSON6 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON6, "name", cJSON_CreateString("Mac (STA)"));
        esp_read_mac(macs, ESP_MAC_WIFI_STA);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/macs[0], /*1*/macs[1], /*2*/macs[2], /*3*/macs[3], /*4*/macs[4], /*5*/macs[5]);
        cJSON_AddItemToObject(entryJSON6, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entryJSON6);

        // WiFi-station (IP address)
        cJSON* entryJSON9 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON9, "name", cJSON_CreateString("WiFi (STA IPv4)"));
        esp_netif_ip_info_t wifi_ip_sta;
        memset(&wifi_ip_sta, 0, sizeof(wifi_ip_sta));
        WifiMgr::getI().GetWiFiSTAIP(wifi_ip_sta);
        sprintf(buff, IPSTR, IP2STR(&wifi_ip_sta.ip));
        cJSON_AddItemToObject(entryJSON9, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entryJSON9);

        esp_ip6_addr_t if_ip6[CONFIG_LWIP_IPV6_NUM_ADDRESSES];
        memset(&if_ip6[0], 0, sizeof(if_ip6));
        const int32_t ipv6_count = WifiMgr::getI().GetWiFiSTAIPv6(if_ip6);
        for(int i = 0; i < MISCMACRO_MIN(ipv6_count, 2); i++)
        {
            char ipv6_string[45+1] = {0,};
            snprintf(ipv6_string, sizeof(ipv6_string)-1, IPV6STR, IPV62STR(if_ip6[i]));

            cJSON* entryJSONIPv6 = cJSON_CreateObject();
            cJSON_AddItemToObject(entryJSONIPv6, "name", cJSON_CreateString("WiFi (STA IPv6)"));
            cJSON_AddItemToObject(entryJSONIPv6, "value", cJSON_CreateString(ipv6_string));
            cJSON_AddItemToArray(entries, entryJSONIPv6);
        }

        // Soft Access point
        // WiFi-AP
        cJSON* entryJSON5 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON5, "name", cJSON_CreateString("Mac (Soft-AP)"));
        esp_read_mac(macs, ESP_MAC_WIFI_SOFTAP);
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", /*0*/macs[0], /*1*/macs[1], /*2*/macs[2], /*3*/macs[3], /*4*/macs[4], /*5*/macs[5]);
        cJSON_AddItemToObject(entryJSON5, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entryJSON5);

        // WiFi-Soft AP (IP address)
        cJSON* entryJSON10 = cJSON_CreateObject();
        cJSON_AddItemToObject(entryJSON10, "name", cJSON_CreateString("WiFi (Soft-AP)"));
        esp_netif_ip_info_t wifi_ip_soft_ap;
        memset(&wifi_ip_soft_ap, 0, sizeof(wifi_ip_soft_ap));
        WifiMgr::getI().GetWiFiSoftAPIP(wifi_ip_soft_ap);
        sprintf(buff, IPSTR, IP2STR(&wifi_ip_soft_ap.ip));
        cJSON_AddItemToObject(entryJSON10, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entryJSON10);

        // Memory
        cJSON* entry_memory_json = cJSON_CreateObject();
        cJSON_AddItemToObject(entry_memory_json, "name", cJSON_CreateString("Memory"));
        const int free_size = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        const int total_size = heap_caps_get_total_size(MALLOC_CAP_8BIT);

        sprintf(buff, "%d / %d", /*0*/(total_size - free_size), /*1*/total_size);
        cJSON_AddItemToObject(entry_memory_json, "value", cJSON_CreateString(buff));
        cJSON_AddItemToArray(entries, entry_memory_json);

        // Current time
        cJSON* entry_curr_time_json = cJSON_CreateObject();
        cJSON_AddItemToObject(entry_curr_time_json, "name", cJSON_CreateString("Time"));
        cJSON_AddItemToObject(entry_curr_time_json, "value", cJSON_CreateString(esp_log_system_timestamp()));
        cJSON_AddItemToArray(entries, entry_curr_time_json);

        // Uptime (s)
        cJSON* entry_up_time_json = cJSON_CreateObject();
        cJSON_AddItemToObject(entry_up_time_json, "name", cJSON_CreateString("Uptime (s)"));
        cJSON_AddItemToObject(entry_up_time_json, "value", cJSON_CreateNumber(esp_log_timestamp()/1000));
        cJSON_AddItemToArray(entries, entry_up_time_json);

        char* str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        return str;
    }
    ERROR:
    cJSON_Delete(root);
    return nullptr;
}

char* WebServer::GetAllSoundLists()
{
    cJSON* root = nullptr;
    {
        root = cJSON_CreateObject();
        if (nullptr == root)
        {
            goto ERROR;
        }

        cJSON* entries = cJSON_AddArrayToObject(root, "files");
        for(int32_t i = 0; i < SoundFX::getI().GetFileCount(); i++)
        {
            const SoundFX::SoundFile* sound_file = SoundFX::getI().GetFile((SoundFX::FileID)i);

            cJSON* new_file = cJSON_CreateObject();
            cJSON_AddItemToObject(new_file, "id", cJSON_CreateNumber(i));
            cJSON_AddItemToObject(new_file, "name", cJSON_CreateString(sound_file->name));
            cJSON_AddItemToObject(new_file, "desc", cJSON_CreateString(sound_file->desc));
            cJSON_AddItemToArray(entries, new_file);
        }
        char* str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        return str;
    }
    ERROR:
    cJSON_Delete(root);
    return nullptr;
}

void WebServer::ToHexString(char dst_hex_string[], const uint8_t* data, uint8_t len)
{
    for (uint32_t i = 0; i < len; i++)
        sprintf(dst_hex_string + (i * 2), "%02X", data[i]);
}
