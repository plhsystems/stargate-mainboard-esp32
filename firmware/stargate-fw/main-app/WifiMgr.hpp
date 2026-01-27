#pragma once

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/ip4_addr.h"
#include "lwip/apps/netbiosns.h"

class WifiMgr
{
    public:
    enum class EState
    {
        Deactivated = 0,
        Connecting = 1,
        Connected = 2,
        Failed = 3,

        Count
    };

    public:
    WifiMgr() = default;
    public:
    // Singleton pattern
    WifiMgr(WifiMgr const&) = delete;
    void operator=(WifiMgr const&) = delete;

    public:
    void Init();

    void Start();

    bool GetWiFiSTAIP(esp_netif_ip_info_t& outIP);
    bool GetWiFiSoftAPIP(esp_netif_ip_info_t& outIP);
    int32_t GetWiFiSTAIPv6(esp_ip6_addr_t if_ip6[CONFIG_LWIP_IPV6_NUM_ADDRESSES]);

    inline EState GetWifiSTAState() const { return m_wifi_sta_state; }

    static WifiMgr& getI()
    {
        static WifiMgr instance;
        return instance;
    }

    private:
    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void wifistation_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    static void time_sync_notification_cb(struct timeval* tv);

    esp_netif_t* m_wifi_sta;
    esp_netif_t* m_wifi_soft_ap;

    EState m_wifi_sta_state = EState::Deactivated;
};

