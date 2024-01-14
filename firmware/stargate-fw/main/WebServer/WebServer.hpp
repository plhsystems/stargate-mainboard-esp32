#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <esp_http_server.h>
class WebServer
{
    /* Max length a file path can have on storage */
    #define HTTPSERVER_BUFFERSIZE (1024*8)

    private:
    WebServer();

    public:
    // Singleton pattern
    WebServer(WebServer const&) = delete;
    void operator=(WebServer const&) = delete;

    public:
    void Init();

    void Start();

    static WebServer& getI()
    {
        static WebServer instance;
        return instance;
    }
    private:
    static esp_err_t file_get_handler(httpd_req_t *req);
    static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

    // Variable
    httpd_handle_t m_server;
    httpd_config_t m_config;

    uint8_t m_u8Buffers[HTTPSERVER_BUFFERSIZE];
};

#endif