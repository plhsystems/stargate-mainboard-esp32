#pragma once

class GateSymbol
{
    public:
    GateSymbol() :
        number(0),
        name("")
    {

    }
    GateSymbol(uint8_t number, const char* name) :
        number(number),
        name(name)
    {

    }
    public:
    const uint8_t number;
    const char* name;
};
