#pragma once

#include <cstring>
#include <cstdio>

class GateAddress
{
    public:
    GateAddress() :
        m_name(""),
        m_symbol_count(0)
    {
        memset(m_symbols, 0, sizeof(m_symbols));
    }

    GateAddress(uint8_t symbols[9], uint8_t symbol_count) :
        m_name(""),
        m_symbol_count(symbol_count)
    {
        memcpy(m_symbols, symbols, sizeof(m_symbols));
    }
    GateAddress(const char* name, uint8_t sym1, uint8_t sym2, uint8_t sym3, uint8_t sym4, uint8_t sym5, uint8_t sym6) :
        m_name(name),
        m_symbol_count(6)
    {
        m_symbols[0] = sym1;
        m_symbols[1] = sym2;
        m_symbols[2] = sym3;

        m_symbols[3] = sym4;
        m_symbols[4] = sym5;
        m_symbols[5] = sym6;

        m_symbols[6] = 0;
        m_symbols[7] = 0;
        m_symbols[8] = 0;
    }

    GateAddress(const char* name, uint8_t sym1, uint8_t sym2, uint8_t sym3, uint8_t sym4, uint8_t sym5, uint8_t sym6, uint8_t sym7) :
        m_name(name),
        m_symbol_count(7)
    {
        m_symbols[0] = sym1;
        m_symbols[1] = sym2;
        m_symbols[2] = sym3;

        m_symbols[3] = sym4;
        m_symbols[4] = sym5;
        m_symbols[5] = sym6;

        m_symbols[6] = sym7;
        m_symbols[7] = 0;
        m_symbols[8] = 0;
    }

    GateAddress(const char* name, uint8_t sym1, uint8_t sym2, uint8_t sym3, uint8_t sym4, uint8_t sym5, uint8_t sym6, uint8_t sym7, uint8_t sym8) :
        m_name(name),
        m_symbol_count(8)
    {
        m_symbols[0] = sym1;
        m_symbols[1] = sym2;
        m_symbols[2] = sym3;

        m_symbols[3] = sym4;
        m_symbols[4] = sym5;
        m_symbols[5] = sym6;

        m_symbols[6] = sym7;
        m_symbols[7] = sym8;
        m_symbols[8] = 0;
    }
    GateAddress(const char* name, uint8_t sym1, uint8_t sym2, uint8_t sym3, uint8_t sym4, uint8_t sym5, uint8_t sym6, uint8_t sym7, uint8_t sym8, uint8_t sym9) :
        m_name(name),
        m_symbol_count(9)
    {
        m_symbols[0] = sym1;
        m_symbols[1] = sym2;
        m_symbols[2] = sym3;

        m_symbols[3] = sym4;
        m_symbols[4] = sym5;
        m_symbols[5] = sym6;

        m_symbols[6] = sym7;
        m_symbols[7] = sym8;
        m_symbols[8] = sym9;
    }
    public:
    static constexpr uint32_t ADDRESSTEXT_LEN = 40;

    inline const char* GetName() const { return m_name; }
    inline uint8_t GetSymbol(uint8_t index) const
    {
        if (index >= m_symbol_count)
            return 0;
        return m_symbols[index];
    }
    inline uint32_t GetSymbolCount() const { return m_symbol_count; }

    inline uint32_t GetAddressText(char text[ADDRESSTEXT_LEN+1])
    {
        return snprintf(text, ADDRESSTEXT_LEN, "%d %d %d\t%d %d %d\t%d %d %d",
            (int)m_symbols[0], (int)m_symbols[1], (int)m_symbols[2],
            (int)m_symbols[3], (int)m_symbols[4], (int)m_symbols[5],
            (int)m_symbols[6], (int)m_symbols[7], (int)m_symbols[8]);
    }

    static constexpr uint32_t SYMBOL_COUNT = 9;
    private:

    const char* m_name;
    uint8_t m_symbols[SYMBOL_COUNT] = {0};
    uint32_t m_symbol_count;
};
