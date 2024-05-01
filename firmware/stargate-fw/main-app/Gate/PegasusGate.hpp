#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class PegasusGate : public BaseGate
{
    public:
    PegasusGate();
    private:
    static const int32_t m_s32SymbolCount = 36;

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

    // TODO: Add https://stargate.fandom.com to the thanks list
    static inline const GateAddress m_gateAddresses[] =
    {
        // Point of origin = 19
        { "Asuras", 1, 6, 25, 34, 16, 30, 19 },   // https://stargate.fandom.com/wiki/Asuras
        { "Genii homeworld", 36, 15, 13, 24, 3, 23, 19 }, // https://stargate.fandom.com/wiki/Genii_homeworld
        { "Lantea", 11, 17, 13, 22, 9, 1, 19 },   // https://stargate.fandom.com/wiki/Lantea
        { "Taranian settlement", 1, 15, 29, 2, 13, 23, 19 },  // https://stargate.fandom.com/wiki/Taranian_settlement
        { "Lucius' planet", 1, 23, 30, 32, 20, 18, 19 },   // https://stargate.fandom.com/wiki/Lucius%27_planet
        { "M1B-129", 23, 32, 30, 1, 18, 20, 19 },  // https://stargate.fandom.com/wiki/M1B-129
        { "M65-PL8", 28, 27, 31, 16, 33, 17, 19 }, // https://stargate.fandom.com/wiki/M65-PL8
        { "M4X-337", 2, 5, 15, 8, 31, 33, 19 }, // https://stargate.fandom.com/wiki/M4X-337
        { "Olesia", 1, 8, 2, 32, 9, 21, 19 }, // https://stargate.fandom.com/wiki/Olesia
        { "Vedeena", 27, 35, 29, 22, 10, 13, 19 }, // https://stargate.fandom.com/wiki/Vedeena
        { "Wraith homeworld", 26, 12, 1, 24, 12, 8, 19 },   // https://stargate.fandom.com/wiki/Wraith_homeworld
        // { "Thenora" }, // INCOMPLETE https://stargate.fandom.com/wiki/Thenora
        // { "Talus" }, // INCOMPLETE https://stargate.fandom.com/wiki/Talus_(planet)
        // { "Proculus" },// INCOMPLETE https://stargate.fandom.com/wiki/Proculus
        // { "Lord Protector's Planet" }, // INCOMPLETE https://stargate.fandom.com/wiki/Lord_Protector%27s_Planet
        // { "Ford's Planet" }, // INCOMPLETE https://stargate.fandom.com/wiki/Ford%27s_planet
    };

    static inline const GateSymbol m_symbols[36]
    {
        { 1, "Subido" },
        { 2, "Bydo" },
        { 3, "Robandus" },
        { 4, "Sibbron" },
        { 5, "Once El" },
        { 6, "Laylox" },
        { 7, "Earth" },
        { 8, "Tahnan" },
        { 9, "Setas" },
        { 10, "Amiwill" },
        { 11, "Acjesis" },
        { 12, "Ca Po" },
        { 13, "Arami" },
        { 14, "Aaxel" },
        { 15, "Illume" },
        { 16, "Ramnon" },
        { 17, "Avoniv" },
        { 18, "Lenchan" },
        { 19, "Alura" },
        { 20, "Ecrumig" },
        { 21, "Zamilloz" },
        { 22, "Danami" },
        { 23, "Salma" },
        { 24, "Hacemill" },
        { 25, "Hamlinto" },
        { 26, "Olavii" },
        { 27, "Poco Re" },
        { 28, "Abrin" },
        { 29, "Roehi" },
        { 30, "Sandovi" },
        { 31, "Gilltin" },
        { 32, "Dawnre" },
        { 33, "Elenami" },
        { 34, "Recktic" },
        { 35, "Aldeni" },
        { 36, "Zeo" },
    };
};


