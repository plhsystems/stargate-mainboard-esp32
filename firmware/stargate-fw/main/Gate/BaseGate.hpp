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

    virtual const GateSymbol& GetSymbol(uint8_t u8SymbolNum) const = 0;
    virtual int32_t GetSymbolCount() const = 0;

    virtual const GateAddress& GetAddress(uint32_t u32Index) const { return InvalidGateAddress; }
    virtual int32_t GetAddressCount() const { return 0; }

    static inline const GateSymbol InvalidSymbol{ 0, "Invalid" };
    static inline const GateAddress InvalidGateAddress { "Invalid" };
};


