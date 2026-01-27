#pragma once

#include "cJSON.h"
#include "esp_http_server.h"
#include "EmbeddedFiles.h"
#include "APIURL.hpp"
#include "./Gate/BaseGate.hpp"
#include "HW/SGHW_HAL.hpp"

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
    void Init(SGHW_HAL* sghw_hal);

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
    char* GetStatus();
    char* GetSysInfo();
    char* GetAllSoundLists();

    char* GetGalaxyInfoJSON(GateGalaxy gate_galaxy);

    static void ToHexString(char dst_hex_string[], const uint8_t* data, uint8_t len);

    static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

    static const EF_SFile* GetFile(const char* filename);

    // Variable
    httpd_handle_t m_server;
    httpd_config_t m_config;

    SGHW_HAL* m_sghw_hal = nullptr;

    uint8_t m_buffers[HTTPSERVER_BUFFERSIZE];

    httpd_uri_t m_http_ui;
    httpd_uri_t m_http_get_api;
    httpd_uri_t m_http_post_api;
    // httpd_uri_t m_gate_control_api_post;

    httpd_uri_t m_http_ota_upload_post;
};

