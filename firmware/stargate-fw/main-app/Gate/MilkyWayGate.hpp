#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class MilkyWayGate : public BaseGate
{
    public:
    MilkyWayGate();
    static const int32_t m_s32SymbolCount = 39;

    inline const GateSymbol& GetSymbol(uint8_t u8SymbolNum) const override
    {
        if (u8SymbolNum < 1 || u8SymbolNum > m_s32SymbolCount)
            return BaseGate::InvalidSymbol;
        return m_symbols[u8SymbolNum-1];
    }

    inline int32_t GetSymbolCount() const override { return m_s32SymbolCount; };

    inline const GateAddress& GetAddress(uint32_t u32Index) const override
    {
        if (u32Index >= GetAddressCount())
            return BaseGate::InvalidGateAddress;
        return m_gateAddresses[u32Index];
    }
    inline int32_t GetAddressCount() const override { return sizeof(m_gateAddresses)/sizeof(m_gateAddresses[0]); }

    // TODO: Add them to the "Thanks" section later.
    // https://www.rdanderson.com/stargate/glyphs/index.htm
    // https://github.com/jonnerd154/StargateProject-software/blob/master/config/defaults-milkyway/addresses.json.dist
    static inline const GateAddress m_gateAddresses[] =
    {
        // Point of origin = 1
        { "ABYDOS", 27, 7, 15, 32, 12, 30, 1 },
        { "APOPHIS'S BASE", 20, 18, 11, 38, 10, 32, 1 },
        { "CASTIANA or SAHAL", 29, 3, 6, 9, 12, 16, 1 },
        { "SAHAL or CASTIANA", 29, 18, 19, 20, 21, 22, 1 },
        { "CHULAK", 9, 2, 23, 15, 37, 20, 1 },
        { "CLAVA THESSARA INFINITAS", 14, 21, 16, 11, 30, 7, 1 },
        { "CLAVA THESSARA INFINITAS2", 26, 20, 35, 32, 23, 4, 1 },
        { "DESTROYERS", 3, 32, 16, 8, 10, 12, 1 },
        { "EARTH", 28, 26, 5, 36, 11, 29, 1 },
        { "EDORA", 28, 38, 35, 9, 15, 3, 1 },
        { "EURONDA", 30, 27, 9, 7, 18, 16, 1 },
        { "JUNA", 29, 8, 18, 22, 4, 25, 1 },
        { "KALLANA", 6, 16, 8, 3, 26, 25, 1 },
        { "KHEB", 26, 35, 6, 8, 23, 14, 1 },
        { "K'TAU", 18, 2, 30, 12, 26, 33, 1 },
        { "MARTIN LLOYD'S HOMEWORLD", 24, 12, 32, 7, 11, 34, 1 },
        { "NID OFF-WORLD BASE", 38, 28, 15, 35, 3, 19, 1 },
        { "OTHALA", 11, 27, 23, 16, 33, 3, 9, 1 },
        { "P2X-555", 28, 8, 16, 33, 13, 31, 1 },
        { "P34-353J", 38, 9, 28, 15, 35, 3, 1 },
        { "P3W-451", 19, 8, 4, 37, 26, 16, 1 }, // Black Hole
        { "P3X-118", 9, 26, 34, 37, 17, 21, 1 },
        { "P3X-562", 3, 28, 9, 35, 24, 15, 1 },
        { "P9C-372", 25, 8, 18, 34, 4, 22, 1 },
        { "PB5-926", 12, 36, 23, 18, 7, 27, 1 },
        { "PX1-767", 20, 2, 35, 8, 26, 15, 1 },
        { "PRACLARUSH TAONAS", 35, 3, 31, 29, 5, 17, 1 },
        { "SANGREAL PLANET", 30, 19, 34, 9, 33, 18, 1 },
        { "TARTARUS", 33, 28, 23, 26, 16, 31, 1 },
        { "TOLLAN", 6, 33, 27, 37, 11, 18, 1 },
        { "TOLLANA", 4, 34, 8, 22, 18, 25, 1 },
        { "VAGON BREI", 3, 8, 2, 12, 19, 30, 1 },
        { "FINAL DESTINATION", 9, 18, 27, 15, 21, 36, 1 },
        { "LANTIA", 19, 21, 2, 16, 15, 8, 20, 1 },
        { "DESTINY", 6, 17, 21, 31, 35, 24, 5, 14, 1 },
    };

    static inline const GateSymbol m_symbols[m_s32SymbolCount] =
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


