#pragma once

#include <stdint.h>
#include "../Common/Chevron.hpp"
#include "GateAddress.hpp"
#include "GateSymbol.hpp"

enum class GateGalaxy
{
    MilkyWay = 0,
    Pegasus = 1,
    Universe = 2,

    Count
};

class BaseGate
{
    public:
     protected:
    BaseGate(GateGalaxy eGateGalaxy);
    public:
    const GateGalaxy eGateGalaxy;

    virtual const GateSymbol& GetSymbol(uint8_t u8SymbolNum) = 0;
    virtual int32_t GetSymbolCount() = 0;

    virtual const GateAddress& GetAddress(int32_t s32Index) { return InvalidGateAddress; }
    virtual int32_t GetAddressCount() { return 0; }

    virtual void LockChevron();
    virtual void UnlockChevron();

    virtual void GoHome();
    virtual void Calibrate();

    virtual void LightUpChevron(Chevron eChevron, bool bIsActive);
    virtual void LightUpSymbol(uint8_t u8Symbol, bool bIsActive);   // Universe

    virtual void MoveToSymbol(uint8_t u8Symbol, Chevron eChevron);

    virtual void ShutdownGate();

    virtual void UnlockGate();
    virtual void LockGate();

    static inline const GateSymbol InvalidSymbol{ 0, "Invalid" };
    static inline const GateAddress InvalidGateAddress { "Invalid" };
};


