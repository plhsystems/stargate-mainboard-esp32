#include "WebServer.hpp"
#include "esp_log.h"
#include <stdio.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_ota_ops.h"
#include "cJSON.h"
#include "../FWConfig.hpp"
#include "misc-macro.h"

#define TAG "webserver"

#define DEFAULT_RELATIVE_URI "/setup/index.html"

WebServer::WebServer()
    : m_server(nullptr)
{
    // API
    m_http_get_api.uri       = "/api/*";
    m_http_get_api.method    = HTTP_GET;
    m_http_get_api.handler   = WebAPIGetHandler;
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    m_http_get_api.user_ctx  = nullptr;

    m_http_post_api.uri       = "/api/*";
    m_http_post_api.method    = HTTP_POST;
    m_http_post_api.handler   = WebAPIPostHandler;
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    m_http_post_api.user_ctx = nullptr;

    // OTA
    m_http_ota_upload_post.uri       = APIURL_POST_OTAUPLOAD_URI;
    m_http_ota_upload_post.method    = HTTP_POST;
    m_http_ota_upload_post.handler   = OTAUploadPostHandler;
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    m_http_ota_upload_post.user_ctx  = nullptr;

    // Get web files
    m_http_ui.uri       = "/*";
    m_http_ui.method    = HTTP_GET;
    m_http_ui.handler   = file_get_handler;
        /* Let's pass response string in user
        * context to demonstrate it's usage */
    m_http_ui.user_ctx  = nullptr;
}

void WebServer::Init(SGHW_HAL* sghw_hal)
{
    m_sghw_hal = sghw_hal;

    m_config = HTTPD_DEFAULT_CONFIG();
    m_config.lru_purge_enable = true;
    m_config.uri_match_fn = httpd_uri_match_wildcard;
    m_config.max_open_sockets = 13;
    m_config.task_priority = FWCONFIG_WEBSERVERTASK_PRIORITY_DEFAULT;
}

void WebServer::Start()
{
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", m_config.server_port);
    if (httpd_start(&m_server, &m_config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(m_server, &m_http_ota_upload_post);
        httpd_register_uri_handler(m_server, &m_http_post_api);
        httpd_register_uri_handler(m_server, &m_http_get_api);
        httpd_register_uri_handler(m_server, &m_http_ui);
    }
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension */
esp_err_t WebServer::set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    } else if (IS_FILE_EXT(filename, ".css")) {
        return httpd_resp_set_type(req, "text/css");
    } else if (IS_FILE_EXT(filename, ".txt")) {
        return httpd_resp_set_type(req, "text/plain");
    } else if (IS_FILE_EXT(filename, ".js")) {
        return httpd_resp_set_type(req, "text/javascript");
    } else if (IS_FILE_EXT(filename, ".json")) {
        return httpd_resp_set_type(req, "application/json");
    } else if (IS_FILE_EXT(filename, ".ttf")) {
        return httpd_resp_set_type(req, "application/x-font-truetype");
    } else if (IS_FILE_EXT(filename, ".woff")) {
        return httpd_resp_set_type(req, "application/font-woff");
    } else if (IS_FILE_EXT(filename, ".html") || IS_FILE_EXT(filename, ".htm")) {
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg") || IS_FILE_EXT(filename, ".jpg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".svg")) {
        return httpd_resp_set_type(req, "image/svg+xml");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    ESP_LOGW(TAG, "Warning, unsupported file extension, returning text/plain as default");
    return httpd_resp_set_type(req, "text/plain");
}

/* An HTTP GET handler */
esp_err_t WebServer::file_get_handler(httpd_req_t *req)
{
    // Redirect root to index.html
    if (strcmp(req->uri, "/") == 0)
    {
        // Remember, browser keep 301 in cache so be careful
        ESP_LOGW(TAG, "Redirect URI: '%s', to '%s'", req->uri, DEFAULT_RELATIVE_URI);
        // Redirect to default page
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_status(req, "301 Moved Permanently");
        httpd_resp_set_hdr(req, "Location", DEFAULT_RELATIVE_URI);
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Opening file uri: %s", req->uri);

    const EF_SFile* file = GetFile(req->uri+1);
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, file->filename);
    if (EF_ISFILECOMPRESSED(file->flags))
    {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    }

    // Static files have 1h cache by default
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600");

    uint32_t index = 0;

    while(index < file->length)
    {
        const uint32_t n = MISCMACRO_MIN(file->length - index, HTTPSERVER_BUFFERSIZE);

        if (n > 0) {
            /* Send the buffer contents as HTTP response m_buffers */
            if (httpd_resp_send_chunk(req, (char*)(file->start_addr + index), n) != ESP_OK) {
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
               return ESP_FAIL;
           }
        }
        index += n;
    }

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

const EF_SFile* WebServer::GetFile(const char* filename)
{
    for(int i = 0; i < EF_EFILE_COUNT; i++)
    {
        const EF_SFile* file = &EF_g_files[i];
        if (strcmp(file->filename, filename) == 0)
            return file;
    }
    return NULL;
}