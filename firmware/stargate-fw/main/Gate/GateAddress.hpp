#pragma once

#include <cstring>

class GateAddress
{
    public:
    GateAddress(const char* szName) :
        m_szName(szName),
        m_u32SymbolCount(0)
    {
        memset(m_u8Symbols, 0, sizeof(m_u8Symbols));
    }

    GateAddress(const char* szName, uint8_t u8Sym1, uint8_t u8Sym2, uint8_t u8Sym3, uint8_t u8Sym4, uint8_t u8Sym5, uint8_t u8Sym6) :
        m_szName(szName),
        m_u32SymbolCount(6)
    {
        m_u8Symbols[0] = u8Sym1;
        m_u8Symbols[1] = u8Sym2;
        m_u8Symbols[2] = u8Sym3;

        m_u8Symbols[3] = u8Sym4;
        m_u8Symbols[4] = u8Sym5;
        m_u8Symbols[5] = u8Sym6;
    }

    GateAddress(const char* szName, uint8_t u8Sym1, uint8_t u8Sym2, uint8_t u8Sym3, uint8_t u8Sym4, uint8_t u8Sym5, uint8_t u8Sym6, uint8_t u8Sym7) :
        m_szName(szName),
        m_u32SymbolCount(7)
    {
        m_u8Symbols[0] = u8Sym1;
        m_u8Symbols[1] = u8Sym2;
        m_u8Symbols[2] = u8Sym3;

        m_u8Symbols[3] = u8Sym4;
        m_u8Symbols[4] = u8Sym5;
        m_u8Symbols[5] = u8Sym6;

        m_u8Symbols[6] = u8Sym7;
    }

    GateAddress(const char* szName, uint8_t u8Sym1, uint8_t u8Sym2, uint8_t u8Sym3, uint8_t u8Sym4, uint8_t u8Sym5, uint8_t u8Sym6, uint8_t u8Sym7, uint8_t u8Sym8) :
        m_szName(szName),
        m_u32SymbolCount(8)
    {
        m_u8Symbols[0] = u8Sym1;
        m_u8Symbols[1] = u8Sym2;
        m_u8Symbols[2] = u8Sym3;

        m_u8Symbols[3] = u8Sym4;
        m_u8Symbols[4] = u8Sym5;
        m_u8Symbols[5] = u8Sym6;

        m_u8Symbols[6] = u8Sym7;
        m_u8Symbols[7] = u8Sym8;
        m_u8Symbols[8] = 0;
    }
    GateAddress(const char* szName, uint8_t u8Sym1, uint8_t u8Sym2, uint8_t u8Sym3, uint8_t u8Sym4, uint8_t u8Sym5, uint8_t u8Sym6, uint8_t u8Sym7, uint8_t u8Sym8, uint8_t u8Sym9) :
        m_szName(szName),
        m_u32SymbolCount(9)
    {
        m_u8Symbols[0] = u8Sym1;
        m_u8Symbols[1] = u8Sym2;
        m_u8Symbols[2] = u8Sym3;

        m_u8Symbols[3] = u8Sym4;
        m_u8Symbols[4] = u8Sym5;
        m_u8Symbols[5] = u8Sym6;

        m_u8Symbols[6] = u8Sym7;
        m_u8Symbols[7] = u8Sym8;
        m_u8Symbols[8] = u8Sym9;
    }
    public:
    inline const char* GetName() const { return m_szName; }
    inline uint8_t GetSymbol(uint8_t u8Index) const
    {
        if (u8Index >= m_u32SymbolCount)
            return 0;
        return m_u8Symbols[u8Index];
    }
    inline uint32_t GetSymbolCount() const { return m_u32SymbolCount; }
    private:
    const char* m_szName;
    uint8_t m_u8Symbols[9];
    uint32_t m_u32SymbolCount;
};
