#include "WifiMgr.hpp"
#include "FWConfig.hpp"
#include "Settings.hpp"
#include "esp_sntp.h"

const char *TAG = "wifi";

WifiMgr::WifiMgr()
{

}

void WifiMgr::wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station %02x:%02x:%02x:%02x:%02x:%02x join, AID=%d",
                 event->mac[0], event->mac[1],event->mac[2], event->mac[3],event->mac[4], event->mac[5],
                 (int)event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station %02x:%02x:%02x:%02x:%02x:%02x leave, AID=%d",
                 event->mac[0], event->mac[1],event->mac[2], event->mac[3],event->mac[4], event->mac[5],
                 (int)event->aid);
    }
}

void WifiMgr::wifistation_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
     }
     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED ) {
        ESP_LOGI(TAG, "Connected to the AP");
        esp_netif_create_ip6_linklocal(getI().m_pWifiSTA);
     } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        //TODO add a timer
        esp_wifi_connect();
        ESP_LOGI(TAG, "connect to the AP faile, retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6) {
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(TAG, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
    }
}

void WifiMgr::Init()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    const bool isWiFiSTA = Settings::getI().GetValueInt32(Settings::Entry::WSTAIsActive) == 1;
    if (isWiFiSTA)
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
    }

    // Access point mode
    esp_netif_t* pWifiSoftAP = esp_netif_create_default_wifi_ap();

    esp_netif_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 192, 168, 4, 1);
	IP4_ADDR(&ipInfo.gw, 192, 168, 4, 1);
	IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
	esp_netif_dhcps_stop(pWifiSoftAP);
	esp_netif_set_ip_info(pWifiSoftAP, &ipInfo);
	esp_netif_dhcps_start(pWifiSoftAP);

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_configAP;
    memset(&wifi_configAP, 0, sizeof(wifi_configAP));
    //wifi_configAP.ap.channel = FWCONFIG_SOFTAP_WIFI_CHANNEL;
    strcpy((char*)wifi_configAP.ap.password, FWCONFIG_SOFTAP_WIFI_PASS);
    wifi_configAP.ap.max_connection = FWCONFIG_SOFTAP_MAX_CONN;
    wifi_configAP.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    uint8_t macAddr[6];
    esp_read_mac(macAddr, ESP_MAC_WIFI_SOFTAP);

    sprintf((char*)wifi_configAP.ap.ssid, FWCONFIG_SOFTAP_WIFI_SSID_BASE, macAddr[3], macAddr[4], macAddr[5]);

    int n = strlen((const char*)wifi_configAP.ap.ssid);
    wifi_configAP.ap.ssid_len = n;

    if (strlen((const char*)FWCONFIG_SOFTAP_WIFI_PASS) == 0) {
        wifi_configAP.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_configAP));

    // Soft Access Point Mode
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             wifi_configAP.ap.ssid, FWCONFIG_SOFTAP_WIFI_PASS);

    if (isWiFiSTA)
    {
        m_pWifiSTA = esp_netif_create_default_wifi_sta();

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        esp_event_handler_instance_t instance_got_ip6;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifistation_event_handler,
                                                            NULL,
                                                            &instance_any_id));

        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &wifistation_event_handler,
                                                            NULL,
                                                            &instance_got_ip));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_GOT_IP6,
                                                            &wifistation_event_handler,
                                                            NULL,
                                                            &instance_got_ip6));

        wifi_config_t wifi_configSTA;
        memset(&wifi_configSTA, 0, sizeof(wifi_configSTA));
        //wifi_configSTA.sta.threshold.authmode = WIFI_AUTH_OPEN;
        wifi_configSTA.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;
        wifi_configSTA.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

        size_t staSSIDLength = 32;
        Settings::getI().GetValueString(Settings::Entry::WSTASSID, (char*)wifi_configSTA.sta.ssid, &staSSIDLength);
        size_t staPassLength = 64;
        Settings::getI().GetValueString(Settings::Entry::WSTAPass, (char*)wifi_configSTA.sta.password, &staPassLength);

        ESP_LOGI(TAG, "STA mode is active, attempt to connect to ssid: %s", wifi_configSTA.sta.ssid);

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_configSTA) );
    }

    // SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_init();
}

void WifiMgr::Start()
{
    // Start AP + STA
    ESP_ERROR_CHECK(esp_wifi_start() );
}

void WifiMgr::time_sync_notification_cb(struct timeval* tv)
{
    // settimeofday(tv, NULL);
    ESP_LOGI(TAG, "Notification of a time synchronization event, sec: %d", (int)tv->tv_sec);

    // Set timezone to Eastern Standard Time and print local time
    time_t now = 0;
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(struct tm));
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "The current date/time in New York is: %2d:%2d:%2d", (int)timeinfo.tm_hour, (int)timeinfo.tm_min, (int)timeinfo.tm_sec);
}
