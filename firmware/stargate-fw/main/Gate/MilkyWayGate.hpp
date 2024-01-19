#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class MilkyWayGate : public BaseGate
{
    public:
    MilkyWayGate();
    static const int32_t m_s32SymbolCount = 39;

    const Symbol& GetSymbol(uint8_t u8SymbolNum)
    {
        if (u8SymbolNum < 1 || u8SymbolNum > m_s32SymbolCount)
            return BaseGate::InvalidSymbol;
        return m_symbols[u8SymbolNum-1];
    }

    inline int32_t GetSymbolCount() { return m_s32SymbolCount; };

    static inline const Symbol m_symbols[m_s32SymbolCount] =
    {
        { 1, "Point of origin (Giza)" },
        { 2, "Crater" },
        { 3, "Virgo" },
        { 4, "Bootes" },
        { 5, "Centaurus" },
        { 6, "Libra" },
        { 7, "Serpens Caput" },
        { 8, "Norma" },
        { 9, "Scorpius" },
        { 10, "Corona Australis" },
        { 11, "Scutum" },
        { 12, "Sagittarius" },
        { 13, "Aquila" },
        { 14, "Microscopium" },
        { 15, "Capricornus" },
        { 16, "Piscis Austrinus" },
        { 17, "Equuleus" },
        { 18, "Aquarius" },
        { 19, "Pegasus" },
        { 20, "Sculptor" },
        { 21, "Pisces" },
        { 22, "Andromeda" },
        { 23, "Triangulum" },
        { 24, "Aries" },
        { 25, "Perseus" },
        { 26, "Cetus" },
        { 27, "Taurus" },
        { 28, "Auriga" },
        { 29, "Eridanus" },
        { 30, "Orion" },
        { 31, "Canis Minor" },
        { 32, "Monoceros" },
        { 33, "Gemini" },
        { 34, "Hydra" },
        { 35, "Lynx" },
        { 36, "Cancer" },
        { 37, "Sextans" },
        { 38, "Leo Minor" },
        { 39, "Leo" },
    };
};


