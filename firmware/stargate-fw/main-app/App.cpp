#include <stdio.h>
#include "esp_log.h"
#include "Gate/GateControl.hpp"
#include "Audio/SoundFX.hpp"
#include "Ring/RingBLEClient.hpp"
#include "WifiMgr.hpp"
#include "Settings.hpp"
#include "HttpClient.hpp"
#include "App.hpp"

#define TAG "MainApp"

void App::Init(Config* config)
{
    m_config = config;
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
    m_config->m_sghw_hal->Init();
    ESP_LOGI(TAG, "Loading sound FX");
    SoundFX::getI().Init();
    ESP_LOGI(TAG, "Initialize WiFi Manager");
    WifiMgr::getI().Init();
    ESP_LOGI(TAG, "Initialize web server");
    WebServer::getI().Init(m_config->m_sghw_hal);
    ESP_LOGI(TAG, "Initialize gate control");
    GateControl::getI().Init(config->m_sghw_hal);
    ESP_LOGI(TAG, "Loading ring BLE communication");
    RingBLEClient::getI().Init();
    ESP_LOGI(TAG, "HTTP Client for external calls");
    HttpClient::getI().Init();

    ESP_LOGI(TAG, "Starting Wi-Fi");
    WifiMgr::getI().Start();
    ESP_LOGI(TAG, "Starting ring BLE communication");
    RingBLEClient::getI().Start();
    ESP_LOGI(TAG, "Starting sound FX");
    SoundFX::getI().Start();
    ESP_LOGI(TAG, "Starting gate control");
    GateControl::getI().StartTask();
    ESP_LOGI(TAG, "Starting web server");
    WebServer::getI().Start();
    ESP_LOGI(TAG, "Starting HTTP Client");
    HttpClient::getI().Start();

    // For debug purpose ...
    char* all_task = (char*)malloc(4096);
    vTaskList(all_task);
    ESP_LOGI(TAG, "vTaskList: \r\n\r\n%s", all_task);
    free(all_task);

    // Autocalibrate as the default action
    // GateControl::getI().QueueAutoHome();
}

void App::LoopTick()
{
    bool sanity = false;
    // The least interesting task to ever exist.
    m_config->m_sghw_hal->SetSanityLED(sanity);
    sanity = !sanity;
    vTaskDelay(pdMS_TO_TICKS(250));
}