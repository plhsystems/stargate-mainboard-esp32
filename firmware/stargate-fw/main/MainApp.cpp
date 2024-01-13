#include <stdio.h>
#include "GateControl.hpp"
#include "esp_log.h"
#include "HW/BoardHW.hpp"

extern "C" {
    void app_main(void);
}

#define TAG "MainApp"

static GateControl m_gc;

void app_main(void)
{
    BoardHW::Init();

    ESP_LOGI(TAG, "Initialize gate control");
    m_gc.Init();
    ESP_LOGI(TAG, "Starting gatecontrol task");
    m_gc.StartTask();

    // Die.
}
