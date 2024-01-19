#pragma once

#include <stdint.h>
#include "../Common/Chevron.hpp"

class Symbol
{
    public:
    Symbol(uint8_t u8Number, const char* szName) :
        u8Number(u8Number),
        szName(szName)
    {

    }
    private:
    const uint8_t u8Number;
    const char* szName;
};

class BaseGate
{
    public:
    enum class Type
    {
        MilkyWay = 0,
        Pegasus = 1,
        Universe = 2,

        Count
    };

    protected:
    BaseGate(Type eType, uint8_t u8Maximum);
    public:
    const Type eType;
    const uint8_t u8Maximum;

    virtual const Symbol& GetSymbol(uint8_t u8SymbolNum) = 0;
    inline int32_t GetSymbolCount() { return u8Maximum; }

    virtual void LockChevron();
    virtual void UnlockChevron();

    virtual void GoHome();
    virtual void Calibrate();

    virtual void LightUpChevron(Chevron eChevron, bool bIsActive);
    virtual void LightUpSymbol(uint8_t u8Symbol, bool bIsActive);   // Universe

    virtual void MoveToSymbol(uint8_t u8Symbol, Chevron eChevron);

    virtual void ShutdownGate();

    virtual void UnlockGate();
    virtual void LockGate();
};


