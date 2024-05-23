#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "App.hpp"
#include "PabloSGHW.hpp"

extern "C" {
    void app_main(void);
}

#define TAG "MainApp"

static App g_app;
static PabloSGHW m_sPabloSGHW;

static App::Config m_sConfig =
{
  .pSGHWHal = &m_sPabloSGHW
};

void app_main(void)
{
  ESP_LOGI(TAG, "app-main init");
  g_app.Init(&m_sConfig);
  ESP_LOGI(TAG, "app-main init, done");

  while(true)
  {
    g_app.LoopTick();
  }
}
