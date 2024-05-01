#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "App.hpp"

extern "C" {
    void app_main(void);
}

#define TAG "MainApp"

static App g_app;

void app_main(void)
{
  g_app.Init();

  while(true)
  {
    g_app.LoopTick();
  }
}
