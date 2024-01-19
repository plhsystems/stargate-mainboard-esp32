#pragma once

class GateSymbol
{
    public:
    GateSymbol(uint8_t u8Number, const char* szName) :
        u8Number(u8Number),
        szName(szName)
    {

    }
    public:
    const uint8_t u8Number;
    const char* szName;
};
