#pragma once

#include <stdint.h>
#include "Chevron.hpp"
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
    BaseGate(GateGalaxy gate_galaxy, const char* name);
    public:
    const GateGalaxy gate_galaxy;
    const char* name;

    virtual const GateSymbol& GetSymbol(uint8_t symbol_num) const = 0;
    virtual int32_t GetSymbolCount() const = 0;

    virtual const GateAddress& GetAddress(uint32_t index) const { return InvalidGateAddress; }
    virtual int32_t GetAddressCount() const { return 0; }

    static inline const GateSymbol InvalidSymbol{ };
    static inline const GateAddress InvalidGateAddress { };
};


