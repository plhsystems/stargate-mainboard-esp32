#pragma once

#include <stdint.h>
#include "BaseGate.hpp"

class PegasusGate : public BaseGate
{
    public:
    PegasusGate();
    private:
    static const int32_t m_s32SymbolCount = 36;

    const GateSymbol& GetSymbol(uint8_t u8SymbolNum)
    {
        if (u8SymbolNum < 1 || u8SymbolNum > m_s32SymbolCount)
            return BaseGate::InvalidSymbol;
        return m_symbols[u8SymbolNum-1];
    }

    inline int32_t GetSymbolCount() { return m_s32SymbolCount; };

    virtual const GateAddress& GetAddress(int32_t s32Index)
    {
        if (s32Index >= GetAddressCount())
            return BaseGate::InvalidGateAddress;
        return m_gateAddresses[s32Index];
    }
    virtual int32_t GetAddressCount() { return sizeof(m_gateAddresses)/sizeof(m_gateAddresses[0]); }

    static inline const GateAddress m_gateAddresses[] =
    {

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


