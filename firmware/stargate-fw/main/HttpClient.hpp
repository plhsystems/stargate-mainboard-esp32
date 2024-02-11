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

    static HttpClient& getI()
    {
        static HttpClient instance;
        return instance;
    }
    private:
    TaskHandle_t m_sTaskHTTPClientHandle;
};

#endif
