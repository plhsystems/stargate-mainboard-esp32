#include "WebServer.hpp"
//#include "HelperMacro.h"
#include "esp_log.h"
//#include "esp_vfs.h"
#include <stdio.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_ota_ops.h"
#include "cJSON.h"
//#include "Settings.h"
#include "../FWConfig.hpp"
//#include "Main.h"
//#include "ApiURL.h"
#include "WebAPI.hpp"
#include "../FWConfig.hpp"

#define TAG "webserver"

#define DEFAULT_RELATIVE_URI "/index.html"

WebServer::WebServer()
    : m_server(nullptr)
{
}

void WebServer::Init()
{
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
        // return server;
    }
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension */
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
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
