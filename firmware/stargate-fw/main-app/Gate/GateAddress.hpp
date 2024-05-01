#pragma once

#include <cstring>
#include <cstdio>

class GateAddress
{
    public:
    GateAddress() :
        m_szName(""),
        m_u32SymbolCount(0)
    {
        memset(m_u8Symbols, 0, sizeof(m_u8Symbols));
    }

    GateAddress(uint8_t u8Symbols[9], uint8_t u8SymbolCount) :
        m_szName(""),
        m_u32SymbolCount(u8SymbolCount)
    {
        memcpy(m_u8Symbols, u8Symbols, sizeof(m_u8Symbols));
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

        m_u8Symbols[6] = 0;
        m_u8Symbols[7] = 0;
        m_u8Symbols[8] = 0;
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
        m_u8Symbols[7] = 0;
        m_u8Symbols[8] = 0;
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
    static constexpr uint32_t ADDRESSTEXT_LEN = 40;

    inline const char* GetName() const { return m_szName; }
    inline uint8_t GetSymbol(uint8_t u8Index) const
    {
        if (u8Index >= m_u32SymbolCount)
            return 0;
        return m_u8Symbols[u8Index];
    }
    inline uint32_t GetSymbolCount() const { return m_u32SymbolCount; }

    inline uint32_t GetAddressText(char szText[ADDRESSTEXT_LEN+1])
    {
        return snprintf(szText, ADDRESSTEXT_LEN, "%d %d %d\t%d %d %d\t%d %d %d",
            (int)m_u8Symbols[0], (int)m_u8Symbols[1], (int)m_u8Symbols[2],
            (int)m_u8Symbols[3], (int)m_u8Symbols[4], (int)m_u8Symbols[5],
            (int)m_u8Symbols[6], (int)m_u8Symbols[7], (int)m_u8Symbols[8]);
    }

    static constexpr uint32_t SYMBOL_COUNT = 9;
    private:

    const char* m_szName;
    uint8_t m_u8Symbols[SYMBOL_COUNT] = {0};
    uint32_t m_u32SymbolCount;
};
