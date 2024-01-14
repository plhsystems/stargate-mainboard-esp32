#include <stdio.h>
#include "GateControl.hpp"
#include "HW/BoardHW.hpp"
#include "WifiMgr.hpp"
#include "esp_log.h"
#include "nvs_flash.h"

extern "C" {
    void app_main(void);
}

#define TAG "MainApp"

static GateControl m_gc;

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    BoardHW::Init();
    WifiMgr::getI().Init();
    ESP_LOGI(TAG, "Initialize gate control");
    m_gc.Init();
    WifiMgr::getI().Start();
    ESP_LOGI(TAG, "Starting gatecontrol task");
    m_gc.StartTask();

    // Die.
}
