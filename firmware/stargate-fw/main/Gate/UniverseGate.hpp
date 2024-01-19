#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class UniverseGate : public BaseGate
{
    public:
    UniverseGate();
    private:
    static const int32_t m_s32SymbolCount = 36;

    const Symbol& GetSymbol(uint8_t u8SymbolNum) { return m_symbols[u8SymbolNum-1]; }
    inline int32_t GetSymbolCount() { return m_s32SymbolCount; };

    static inline const Symbol m_symbols[36]
    {
        // Universe symbols aren't named
        { 1, "" },
        { 2, "" },
        { 3, "" },
        { 4, "" },
        { 5, "" },
        { 6, "" },
        { 7, "" },
        { 8, "" },
        { 9, "" },
        { 10, "" },
        { 11, "" },
        { 12, "" },
        { 13, "" },
        { 14, "" },
        { 15, "" },
        { 16, "" },
        { 17, "" },
        { 18, "" },
        { 19, "" },
        { 20, "" },
        { 21, "" },
        { 22, "" },
        { 23, "" },
        { 24, "" },
        { 25, "" },
        { 26, "" },
        { 27, "" },
        { 28, "" },
        { 29, "" },
        { 30, "" },
        { 31, "" },
        { 32, "" },
        { 33, "" },
        { 34, "" },
        { 35, "" },
        { 36, "" },
    };
};


