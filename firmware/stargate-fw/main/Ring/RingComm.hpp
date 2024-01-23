#ifndef _RINGU_H_
#define _RINGU_H_

#include "stdint.h"
#include "SGUComm.hpp"

class RingComm
{
    public:
    enum class GateAnimation
    {
        FadeIn = 0,
        FadeOut = 1,
        ErrorToWhite = 2,
        ErrorToOff = 3,
        AllSymbolsOn = 4,

        Count
    };
    private:
    RingComm();

    public:
    // Singleton pattern
    RingComm(RingComm const&) = delete;
    void operator=(RingComm const&) = delete;

    public:
    void Init();
    void Start();

    static void TaskRunning(RingComm* pRC);

    void SendKeepAlive();
    void SendPowerOff();
    void SendLightUpSymbol(uint8_t u8Symbol);
    void SendGateAnimation(GateAnimation animation);
    void SendGotoFactory();

    static RingComm& getI()
    {
        static RingComm instance;
        return instance;
    }
    private:
};

#endif
