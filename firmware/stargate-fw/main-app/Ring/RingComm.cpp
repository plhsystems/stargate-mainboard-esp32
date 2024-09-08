#include "RingComm.hpp"
#include "esp_log.h"
#include "SGUComm.hpp"
#include "SGURing.hpp"
#include "../FWConfig.hpp"
#include "../Settings.hpp"

static const char *TAG = "RingComm";

RingComm::RingComm()
{
}

void RingComm::Init()
{
    m_xMutexHandle = xSemaphoreCreateMutexStatic( &m_xMutexBuffer );
}

void RingComm::Start()
{
    LockMutex();
    ESP_LOGI(TAG, "Start ring comm");
    // Create a task to receive communication datas
	if (xTaskCreatePinnedToCore(TaskRunning, "RingComm", FWCONFIG_RINGCOMM_STACKSIZE, (void*)this, FWCONFIG_RINGCOMM_PRIORITY_DEFAULT, &m_sRingCommHandle, FWCONFIG_RINGCOMM_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
    UnlockMutex();
}

void RingComm::TaskRunning(void* pArg)
{
    RingComm* pRC = (RingComm*)pArg;

    pRC->LockMutex();
    pRC->m_commSocket = -1;
    {
    pRC->m_dest_addr.sin_addr.s_addr = inet_addr(FWCONFIG_RING_IPADDRESS);
    pRC->m_dest_addr.sin_family = AF_INET;
    pRC->m_dest_addr.sin_port = htons(FWCONFIG_RING_PORT);

    pRC->m_commSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    pRC->UnlockMutex();
    if (pRC->m_commSocket < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", FWCONFIG_RING_IPADDRESS, FWCONFIG_RING_PORT);

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

                // Receiving response
                pRC->m_u32LastPingResponse = xTaskGetTickCount();
                if (!pRC->m_bIsConnected)
                {
                    pRC->m_bIsConnected = true;
                    ESP_LOGI(TAG, "received %d bytes", len);
                    ESP_LOGI(TAG, "Connected to the ring");
                }
            }
        }

        // Disconnection detected
        if (pRC->m_bIsConnected && pdTICKS_TO_MS(xTaskGetTickCount() - pRC->m_u32LastPingResponse) > PINGPONG_TIMEOUT_MS )
        {
            pRC->m_bIsConnected = false;
            ESP_LOGI(TAG, "Disconnected from the ring");
        }

        // Ping every seconds
        if ( pdTICKS_TO_MS(xTaskGetTickCount() - ttPingPongTicks) > PINGPONG_INTERVAL_MS ) {
            ttPingPongTicks = xTaskGetTickCount();
            const SGUCommNS::SPingPongArg arg = { .u32PingPong = u32PingPong };
            uint8_t payloads[16];
            const uint32_t u32Length = SGUCommNS::SGUComm::EncPingPong(payloads, sizeof(payloads), &arg);
            u32PingPong++;

            int err = sendto(pRC->m_commSocket, payloads, u32Length, 0, (struct sockaddr *)&pRC->m_dest_addr, sizeof(pRC->m_dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during ping: errno %d", errno);
                // goto CLEAN_UP;
            }
        }
        // 50 hz maximum
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    }
    CLEAN_UP:
    pRC->LockMutex();
    if (pRC->m_commSocket != -1) {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(pRC->m_commSocket, 0);
        close(pRC->m_commSocket);
        pRC->m_commSocket = 1;
    }
    pRC->UnlockMutex();
    vTaskDelete(NULL);
}

bool RingComm::GetIsConnected()
{
    return m_bIsConnected;
}

void RingComm::SendPowerOff()
{
    LockMutex();
    uint8_t payloads[16];
    const uint32_t u32Length = SGUCommNS::SGUComm::EncTurnOff(payloads, sizeof(payloads));
    const int err = sendto(m_commSocket, payloads, u32Length, 0, (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    UnlockMutex();
}

void RingComm::SendLightUpSymbol(uint8_t u8Symbol)
{
    LockMutex();
    uint8_t payloads[16];
    const uint8_t u8Lights[] = { (uint8_t)SGURingNS::SymbolToLedIndex(u8Symbol) };

    const uint8_t u8Bright = Settings::getI().GetValueInt32(Settings::Entry::RingSymbolLight);

    const SGUCommNS::SUpdateLightArg arg =
    {
        .sColor = { .u8Red = u8Bright, .u8Green = u8Bright, .u8Blue = u8Bright },
        .u8Lights = u8Lights,
        .u8LightCount = sizeof(u8Lights) / sizeof(uint8_t)
    };
    const uint32_t u32Length = SGUCommNS::SGUComm::EncUpdateLight(payloads, sizeof(payloads), &arg);
    const int err = sendto(m_commSocket, payloads, u32Length, 0, (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    UnlockMutex();
}

void RingComm::SendGateAnimation(SGUCommNS::EChevronAnimation animation)
{
    LockMutex();
    uint8_t payloads[16];
    const SGUCommNS::SChevronsLightningArg arg = { .eChevronAnim = animation };
    const uint32_t u32Length = SGUCommNS::SGUComm::EncChevronLightning(payloads, sizeof(payloads), &arg);
    const int err = sendto(m_commSocket, payloads, u32Length, 0, (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    UnlockMutex();
}

void RingComm::SendGotoFactory()
{
    LockMutex();
    uint8_t payloads[16];
    const uint32_t u32Length = SGUCommNS::SGUComm::EncGotoFactory(payloads, sizeof(payloads));
    const int err = sendto(m_commSocket, payloads, u32Length, 0, (struct sockaddr *)&m_dest_addr, sizeof(m_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    UnlockMutex();
}
