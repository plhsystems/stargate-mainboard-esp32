#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_

#include "esp_log.h"
#include "esp_http_client.h"

class HttpClient
{
    public:

    private:
    HttpClient();

    public:
    // Singleton pattern
    HttpClient(HttpClient const&) = delete;
    void operator=(HttpClient const&) = delete;

    public:
    void Init();

    void Start();

    static void TaskRunning(void* pArg);

    static esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt);

    static HttpClient& getI()
    {
        static HttpClient instance;
        return instance;
    }
    private:
    TaskHandle_t m_sTaskHTTPClientHandle;
    uint8_t m_u8Buffers[1024*10];
};

#endif
