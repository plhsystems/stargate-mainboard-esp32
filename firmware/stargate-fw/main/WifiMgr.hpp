#ifndef _WIFIMGR_H_
#define _WIFIMGR_H_

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/ip4_addr.h"
#include "lwip/apps/netbiosns.h"

class WifiMgr
{
    private:
    WifiMgr();

    public:
    // Singleton pattern
    WifiMgr(WifiMgr const&) = delete;
    void operator=(WifiMgr const&) = delete;

    public:
    void Init();

    void Start();

    static WifiMgr& getI()
    {
        static WifiMgr instance;
        return instance;
    }
    private:
    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void wifistation_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    esp_netif_t* m_pWifiSTA;
};

#endif
