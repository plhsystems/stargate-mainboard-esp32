#include <stdio.h>
#include "GateControl/GateControl.hpp"
#include "HW/HW.hpp"
#include "WebServer/WebServer.hpp"
#include "Audio/SoundFX.hpp"
#include "Ring/RingComm.hpp"
#include "WifiMgr.hpp"
#include "Settings.hpp"
#include "HttpClient.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
    void app_main(void);
}

#define TAG "MainApp"

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Initialize settings");
    Settings::getI().Init();
    ESP_LOGI(TAG, "Loading settings");
    Settings::getI().Load();

    // Initialize all modules
    ESP_LOGI(TAG, "Initialize gate control");
    HW::getI()->Init();
    ESP_LOGI(TAG, "Loading sound FX");
    SoundFX::getI().Init();
    ESP_LOGI(TAG, "Initialize WiFi Manager");
    WifiMgr::getI().Init();
    ESP_LOGI(TAG, "Initialize web server");
    WebServer::getI().Init();
    ESP_LOGI(TAG, "Initialize gate control");
    GateControl::getI().Init();
    ESP_LOGI(TAG, "Loading ring communication");
    RingComm::getI().Init();
    ESP_LOGI(TAG, "HTTP Client for external calls");
    HttpClient::getI().Init();

    ESP_LOGI(TAG, "Starting Wi-Fi");
    WifiMgr::getI().Start();
    ESP_LOGI(TAG, "Starting ring communication");
    RingComm::getI().Start();
    ESP_LOGI(TAG, "Starting sound FX");
    SoundFX::getI().Start();
    ESP_LOGI(TAG, "Starting gate control");
    GateControl::getI().StartTask();
    ESP_LOGI(TAG, "Starting web server");
    WebServer::getI().Start();
    ESP_LOGI(TAG, "Starting HTTP Client");
    HttpClient::getI().Start();

    // For debug purpose ...
    char* szAllTask = (char*)malloc(4096);
    vTaskList(szAllTask);
    ESP_LOGI(TAG, "vTaskList: \r\n\r\n%s", szAllTask);
    free(szAllTask);

    // Autocalibrate as the default action
    GateControl::getI().QueueAction(GateControl::ECmd::AutoHome);

    bool bSanity = false;
    while(true)
    {
      // The least interesting task to ever exist.
      HW::getI()->SetSanityLED(bSanity);
      bSanity = !bSanity;
      vTaskDelay(pdMS_TO_TICKS(250));
    }
}
