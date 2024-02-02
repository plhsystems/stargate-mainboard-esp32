#include "RingComm.hpp"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "../FWConfig.hpp"

static const char *TAG = "RingComm";
static const char *payload = "Message from ESP32 ";

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
    {
    char rx_buffer[128];
    char host_ip[] = "192.168.5.100";
    uint16_t u16Port = 5000;

    struct sockaddr_in dest_addr_ip4;
    dest_addr_ip4.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4.sin_family = AF_INET;
    dest_addr_ip4.sin_port = htons(u16Port);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto ERROR;
    }
    ESP_LOGI(TAG, "Socket created, %s:%d", host_ip, u16Port);
    int err = bind(sock, (struct sockaddr *)&dest_addr_ip4, sizeof(dest_addr_ip4));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto ERROR;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", (int)u16Port);

    while (1) {

        ESP_LOGI(TAG, "sendto");
        /*
        int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr_ip4, sizeof(dest_addr_ip4));
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            goto ERROR;
        }
        ESP_LOGI(TAG, "Message sent");
        */
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        ESP_LOGI(TAG, "recvfrom");

        struct timeval read_timeout;
        read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 10;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));

        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
        ESP_LOGI(TAG, "recvfrom done");

        // Error occurred during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            goto ERROR;
        }
        // Data received
        else {
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
            ESP_LOGI(TAG, "%s", rx_buffer);
            if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                ESP_LOGI(TAG, "Received expected message, reconnecting");
                break;
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    }
    ERROR:
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
