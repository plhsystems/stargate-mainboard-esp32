#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class UniverseGate : public BaseGate
{
    public:
    UniverseGate();
    private:
    static const int32_t m_s32SymbolCount = 36;

    const Symbol& GetSymbol(uint8_t u8SymbolNum)
    {
        if (u8SymbolNum < 1 || u8SymbolNum > m_s32SymbolCount)
            return BaseGate::InvalidSymbol;
        return m_symbols[u8SymbolNum-1];
    }

    inline int32_t GetSymbolCount() { return m_s32SymbolCount; };

    static inline const Symbol m_symbols[36]
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


