#pragma once

#include <memory>
#include "esp_log.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class HttpClient
{
    public:
    HttpClient();

    public:
    // Singleton pattern
    HttpClient(HttpClient const&) = delete;
    void operator=(HttpClient const&) = delete;

    public:
    void Init();

    void Start();

    std::shared_ptr<char[]> GetFanGateListString();

    static void TaskRunning(void* arg);

    static HttpClient& getI()
    {
        static HttpClient instance;
        return instance;
    }

    private:
    TaskHandle_t m_task_http_client_handle;

    SemaphoreHandle_t m_fanGate_mutex;

    std::shared_ptr<char[]> m_fanGate;
    uint32_t m_last_update_ticks;
};

