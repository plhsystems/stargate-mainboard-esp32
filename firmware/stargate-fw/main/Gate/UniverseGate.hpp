#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class UniverseGate : public BaseGate
{
    public:
    UniverseGate();
    private:
    static const int32_t m_s32SymbolCount = 36;

    const GateSymbol& GetSymbol(uint8_t u8SymbolNum) override
    {
        if (u8SymbolNum < 1 || u8SymbolNum > m_s32SymbolCount)
            return BaseGate::InvalidSymbol;
        return m_symbols[u8SymbolNum-1];
    }

    inline int32_t GetSymbolCount() override { return m_s32SymbolCount; };

    const GateAddress& GetAddress(uint32_t u32Index) override
    {
        if (u32Index >= GetAddressCount())
            return BaseGate::InvalidGateAddress;
        return m_gateAddresses[u32Index];
    }
    int32_t GetAddressCount() override { return sizeof(m_gateAddresses)/sizeof(m_gateAddresses[0]); }

    static inline const GateAddress m_gateAddresses[] =
    {
        { "Earth", 10, 15, 20, 26, 28, 13, 18, 32, 30 },
        { "Jungle Planet", 3, 34, 12, 7, 19, 6, 29 },
        { "Desert Planet", 7, 8, 14, 17, 32, 23, 33 },
        { "Hoth", 15, 35, 8, 30, 31, 29, 33 },
        { "Grave Pit Planet", 1, 34, 12, 7, 25, 32, 33 },
        { "Ruins Planet", 10, 12, 14, 23, 32, 23, 33 },
        { "Foggy Planet", 4, 20, 23, 28, 6, 11, 33 },
        { "Deportation Planet", 4, 17, 23, 32, 35, 8, 33 },
        { "Cloverdale Planet", 3, 13, 17, 19, 1, 35, 33 },
        { "Malice Planet",2, 14, 18, 23, 30, 5, 33 },
        { "Novus Colony Planet",5, 17, 23, 25, 1, 28, 33 },
        { "Last Planet", 6, 26, 28, 10, 31, 12, 33 },
    };

    static inline const GateSymbol m_symbols[36]
    {
        // Universe symbols aren't named
        { 1, "1" },
        { 2, "2" },
        { 3, "3" },
        { 4, "4" },
        { 5, "5" },
        { 6, "6" },
        { 7, "7" },
        { 8, "8" },
        { 9, "9" },
        { 10, "10" },
        { 11, "11" },
        { 12, "12" },
        { 13, "13" },
        { 14, "14" },
        { 15, "15" },
        { 16, "16" },
        { 17, "17" },
        { 18, "18" },
        { 19, "19" },
        { 20, "20" },
        { 21, "21" },
        { 22, "22" },
        { 23, "23" },
        { 24, "24" },
        { 25, "25" },
        { 26, "26" },
        { 27, "27" },
        { 28, "28" },
        { 29, "29" },
        { 30, "30" },
        { 31, "31" },
        { 32, "32" },
        { 33, "33" },
        { 34, "34" },
        { 35, "35" },
        { 36, "36" },
    };
};


