#ifndef _RINGU_H_
#define _RINGU_H_

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

    TaskHandle_t m_sRingCommHandle;

    int m_commSocket = -1;
    struct sockaddr_in m_dest_addr;
};

#endif
