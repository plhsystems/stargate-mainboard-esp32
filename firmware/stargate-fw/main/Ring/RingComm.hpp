#pragma once

#include "stdint.h"
#include "SGUComm.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SGUComm.hpp"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

class RingComm
{
    public:
    private:
    RingComm();

    public:
    // Singleton pattern
    RingComm(RingComm const&) = delete;
    void operator=(RingComm const&) = delete;

    public:
    void Init();
    void Start();

    static void TaskRunning(void* pArg);

    void SendKeepAlive();
    void SendPowerOff();
    void SendLightUpSymbol(uint8_t u8Symbol);
    void SendGateAnimation(SGUCommNS::EChevronAnimation animation);
    void SendGotoFactory();

    static RingComm& getI()
    {
        static RingComm instance;
        return instance;
    }
    private:
    bool LockMutex() { return (pdTRUE == xSemaphoreTake( m_xMutexHandle, ( TickType_t ) pdMS_TO_TICKS(100) )); }
    void UnlockMutex() { xSemaphoreGive( m_xMutexHandle ); }

    private:

    // Working delay
    static constexpr uint32_t PINGPONG_TIMEOUT_MS = 1500;
    static constexpr uint32_t PINGPONG_INTERVAL_MS = 500;

    // Task related
    TaskHandle_t m_sRingCommHandle;

    // Socket related
    int m_commSocket = -1;
    struct sockaddr_in m_dest_addr;

    // Process
    uint32_t m_u32LastPingResponse = 0;
    bool m_bIsConnected = false;

    // Mutex
    StaticSemaphore_t m_xMutexBuffer; // Define the buffer for the mutex's data structure
    SemaphoreHandle_t m_xMutexHandle; // Declare a handle for the mutex
};

