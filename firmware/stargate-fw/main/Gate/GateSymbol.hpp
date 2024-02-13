#pragma once

class GateSymbol
{
    public:
    GateSymbol() :
        u8Number(0),
        szName("")
    {

    }
    GateSymbol(uint8_t u8Number, const char* szName) :
        u8Number(u8Number),
        szName(szName)
    {

    }
    public:
    const uint8_t u8Number;
    const char* szName;
};
