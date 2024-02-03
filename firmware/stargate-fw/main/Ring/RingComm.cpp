#include "RingComm.hpp"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "../FWConfig.hpp"

static const char *TAG = "RingComm";

RingComm::RingComm()
{
}

void RingComm::Init()
{

}

void RingComm::Start()
{
    ESP_LOGI(TAG, "Start ring comm");
    // Create a task to receive communication datas
	if (xTaskCreatePinnedToCore(TaskRunning, "RingComm", FWCONFIG_RINGCOMM_STACKSIZE, (void*)this, FWCONFIG_RINGCOMM_PRIORITY_DEFAULT, &m_sRingCommHandle, FWCONFIG_RINGCOMM_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
}

void RingComm::TaskRunning(void* pArg)
{
    RingComm* pRC = (RingComm*)pArg;
    int sock = -1;

    #define HOST_IP_ADDR "192.168.42.250"
    #define PORT 7827

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    while(true)
    {
        const char* payload = "coucou_test";
        int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            goto CLEAN_UP;
        }
        ESP_LOGI(TAG, "Message sent");

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    CLEAN_UP:
    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}

void RingComm::SendPowerOff()
{

}

void RingComm::SendLightUpSymbol(uint8_t u8Symbol)
{

}

void RingComm::SendGateAnimation(GateAnimation animation)
{

}

void RingComm::SendGotoFactory()
{

}

/*

void RingComm::TaskRunning(void* pArg)
{
    RingComm* pRC = (RingComm*)pArg;

    #define KEEPALIVE_IDLE (5)
    #define KEEPALIVE_INTERVAL (5)
    #define KEEPALIVE_COUNT (5)

    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    uint16_t u16Port = 8888;

    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(u16Port);

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", IPPROTO_IP);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", u16Port);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while(true)
    {
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        char addr_str[24];
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        char rx_buffer[128];
        int len = 0;
        do {
            len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            } else if (len == 0) {
                ESP_LOGW(TAG, "Connection closed");
            } else {
                rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
                ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
            }
        } while (len > 0);

        shutdown(sock, 0);
        close(sock);
    }

    CLEAN_UP:
    vTaskDelete(NULL);
}
*/