#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class MilkyWayGate : public BaseGate
{
    public:
    MilkyWayGate();
    static const int32_t m_s32SymbolCount = 39;

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

    // TODO: Add them to the "Thanks" section later.
    // https://www.rdanderson.com/stargate/glyphs/index.htm
    // https://github.com/jonnerd154/StargateProject-software/blob/master/config/defaults-milkyway/addresses.json.dist
    static inline const GateAddress m_gateAddresses[] =
    {
        { "ABYDOS", 27, 7, 15, 32, 12, 30 },
        { "APOPHIS'S BASE", 20, 18, 11, 38, 10, 32 },
        { "CASTIANA or SAHAL", 29, 3, 6, 9, 12, 16 },
        { "SAHAL or CASTIANA", 29, 18, 19, 20, 21, 22 },
        { "CHULAK", 9, 2, 23, 15, 37, 20 },
        { "CLAVA THESSARA INFINITAS", 14, 21, 16, 11, 30, 7 },
        { "CLAVA THESSARA INFINITAS2", 26, 20, 35, 32, 23, 4 },
        { "DESTROYERS", 3, 32, 16, 8, 10, 12 },
        { "EARTH", 28, 26, 5, 36, 11, 29 },
        { "EDORA", 28, 38, 35, 9, 15, 3 },
        { "EURONDA", 30, 27, 9, 7, 18, 16 },
        { "JUNA", 29, 8, 18, 22, 4, 25 },
        { "KALLANA", 6, 16, 8, 3, 26, 25 },
        { "KHEB", 26, 35, 6, 8, 23, 14 },
        { "K'TAU", 18, 2, 30, 12, 26, 33 },
        { "MARTIN LLOYD'S HOMEWORLD", 24, 12, 32, 7, 11, 34 },
        { "NID OFF-WORLD BASE", 38, 28, 15, 35, 3, 19 },
        { "OTHALA", 11, 27, 23, 16, 33, 3, 9 },
        { "P2X-555", 28, 8, 16, 33, 13, 31 },
        { "P34-353J", 38, 9, 28, 15, 35, 3 },
        { "P3W-451", 19, 8, 4, 37, 26, 16 }, // Black Hole
        { "P3X-118", 9, 26, 34, 37, 17, 21 },
        { "P3X-562", 3, 28, 9, 35, 24, 15 },
        { "P9C-372", 25, 8, 18, 34, 4, 22 },
        { "PB5-926", 12, 36, 23, 18, 7, 27 },
        { "PX1-767", 20, 2, 35, 8, 26, 15 },
        { "PRACLARUSH TAONAS", 35, 3, 31, 29, 5, 17 },
        { "SANGREAL PLANET", 30, 19, 34, 9, 33, 18 },
        { "TARTARUS", 33, 28, 23, 26, 16, 31 },
        { "TOLLAN", 6, 33, 27, 37, 11, 18 },
        { "TOLLANA", 4, 34, 8, 22, 18, 25 },
        { "VAGON BREI", 3, 8, 2, 12, 19, 30 },
        { "FINAL DESTINATION", 9, 18, 27, 15, 21, 36 },
        { "LANTIA", 19, 21, 2, 16, 15, 8, 20 },
        { "DESTINY", 6, 17, 21, 31, 35, 24, 5, 14 },
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


