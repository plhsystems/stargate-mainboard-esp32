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
static PinkySGHW m_sPinkySGHW;

static App::Config m_sConfig =
{
  .pSGHWHal = &m_sPinkySGHW
};

void app_main(void)
{
  g_app.Init(&m_sConfig);

  while(true)
  {
    g_app.LoopTick();
  }
}
