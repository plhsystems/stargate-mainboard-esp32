#include "RingComm.hpp"
#include "esp_log.h"
#include "SGUComm.hpp"
#include "SGURing.hpp"
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
    pRC->m_commSocket = -1;
    {
    pRC->m_dest_addr.sin_addr.s_addr = inet_addr(FWCONFIG_RING_IPADDRESS);
    pRC->m_dest_addr.sin_family = AF_INET;
    pRC->m_dest_addr.sin_port = htons(FWCONFIG_RING_PORT);

    pRC->m_commSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (pRC->m_commSocket < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", FWCONFIG_RING_IPADDRESS, FWCONFIG_RING_PORT);

    uint32_t u32 = 0;
    uint32_t u32PingPong = 1;

    TickType_t ttPingPongTicks = 0;

    while(true)
    {
        struct timeval tv = {
            .tv_sec = 0,
            .tv_usec = 50000,
        };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(pRC->m_commSocket, &rfds);
        int s = select(pRC->m_commSocket + 1, &rfds, NULL, NULL, &tv);
        if (s < 0) {
            ESP_LOGE(TAG, "Select failed: errno %d", errno);
            break;
        }
        else if (s > 0) {
            if (FD_ISSET(pRC->m_commSocket, &rfds)) {
                // Incoming datagram received
                char recvbuf[48];

                struct sockaddr_storage raddr; // Large enough for both IPv4 or IPv6
                socklen_t socklen = sizeof(raddr);
                int len = recvfrom(pRC->m_commSocket, recvbuf, sizeof(recvbuf)-1, 0,
                                    (struct sockaddr *)&raddr, &socklen);
                if (len < 0) {
                    ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                    break;
                }
                ESP_LOGI(TAG, "received %d bytes", len);
            }
        }

        // Ping every seconds
        if ( pdTICKS_TO_MS(xTaskGetTickCount() - ttPingPongTicks) > 1000 ) {
            ttPingPongTicks = xTaskGetTickCount();
            const SGUCommNS::SPingPongArg arg = { .u32PingPong = u32PingPong };
            uint8_t payloads[16];
            const int32_t length = SGUCommNS::SGUComm::EncPingPong(payloads, sizeof(payloads), &arg);
            u32PingPong++;

            int err = sendto(pRC->m_commSocket, payloads, length, 0, (struct sockaddr *)&pRC->m_dest_addr, sizeof(pRC->m_dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                goto CLEAN_UP;
            }
        }
        // 50 hz maximum
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    }
    CLEAN_UP:
    if (pRC->m_commSocket != -1) {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(pRC->m_commSocket, 0);
        close(pRC->m_commSocket);
    }
    vTaskDelete(NULL);
}

void RingComm::SendPowerOff()
{
    /*
    const SGUCommNS::SChevronsLightningArg arg = { .eChevronAnim = SGUCommNS::EChevronAnimation::FadeOut };
    length = SGUCommNS::SGUComm::EncChevronLightning(payloads, sizeof(payloads), &arg);
    */
}

void RingComm::SendLightUpSymbol(uint8_t u8Symbol)
{
    uint8_t payloads[16];
    const uint8_t u8Lights[] = { (uint8_t)SGURingNS::SymbolToLedIndex(u8Symbol) };
    const SGUCommNS::SUpdateLightArg arg =
    {
        .sColor = { .u8Red = 15, .u8Green = 15, .u8Blue = 15 },
        .u8Lights = u8Lights,
        .u8LightCount = sizeof(u8Lights) / sizeof(uint8_t)
    };
    const int32_t length = SGUCommNS::SGUComm::EncUpdateLight(payloads, sizeof(payloads), &arg);
    const int err = sendto(m_commSocket, payloads, length, 0, (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}

void RingComm::SendGateAnimation(SGUCommNS::EChevronAnimation animation)
{
    uint8_t payloads[16];
    const SGUCommNS::SChevronsLightningArg arg = { .eChevronAnim = animation };
    const int32_t length = SGUCommNS::SGUComm::EncChevronLightning(payloads, sizeof(payloads), &arg);
    const int err = sendto(m_commSocket, payloads, length, 0, (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}

void RingComm::SendGotoFactory()
{

}
