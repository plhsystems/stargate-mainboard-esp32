#pragma once

#include <memory>
#include "esp_log.h"
#include "esp_http_client.h"

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

    const char* GetFanGateListString();

    static void TaskRunning(void* pArg);

    static HttpClient& getI()
    {
        static HttpClient instance;
        return instance;
    }

    private:
    TaskHandle_t m_sTaskHTTPClientHandle;

    std::shared_ptr<char[]> m_fanGate;
    uint32_t m_u32LastUpdateTicks;
};

