#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "App.hpp"
#include "PinkySGHW.hpp"

extern "C" {
    void app_main(void);
}

#define TAG "MainApp"

static App g_app;
static PinkySGHW m_pinky_sghw;

static App::Config m_config =
{
  .m_sghw_hal = &m_pinky_sghw
};

void app_main(void)
{
  ESP_LOGI(TAG, "app-main init");
  g_app.Init(&m_config);
  ESP_LOGI(TAG, "app-main init, done");

  while(true)
  {
    g_app.LoopTick();
  }
}
