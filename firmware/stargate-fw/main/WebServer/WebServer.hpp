#pragma once

#include "esp_http_server.h"
#include "EmbeddedFiles.h"
#include "APIURL.hpp"
#include "./Gate/BaseGate.hpp"

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
    static esp_err_t OTAUploadPostHandler(httpd_req_t *req);

    static esp_err_t WebAPIGetHandler(httpd_req_t *req);
    static esp_err_t WebAPIPostHandler(httpd_req_t *req);
    static esp_err_t GateControlAPIPostHandler(httpd_req_t *req);

    // Get API
    static char* GetStatus();
    static char* GetSysInfo();
    static char* GetAllSoundLists();

    static char* GetGalaxyInfoJSON(GateGalaxy gateGalaxy);

    static void ToHexString(char dstHexString[], const uint8_t* data, uint8_t len);

    static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

    static const EF_SFile* GetFile(const char* strFilename);

    // Variable
    httpd_handle_t m_server;
    httpd_config_t m_config;

    uint8_t m_u8Buffers[HTTPSERVER_BUFFERSIZE];

    httpd_uri_t m_sHttpUI;
    httpd_uri_t m_sHttpGetAPI;
    httpd_uri_t m_sHttpPostAPI;
    // httpd_uri_t m_sGateControlAPIPost;

    httpd_uri_t m_sHttpOTAUploadPost;
};

