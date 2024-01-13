#pragma once

#include <stdint.h>

class BaseGate
{
    public:
    enum class Type
    {
        Kristian = 0,
        Atlantis = 1,
        Universe = 2,

        Count
    };

    enum class Chevron
    {
        Chevron7_Master = 0,

        Chevron1,
        Chevron2,
        Chevron3,
        Chevron4,
        Chevron5,
        Chevron6,

        Chevron8,
        Chevron9,

        Count
    };

    protected:
    BaseGate(Type eType, uint8_t u8Minimum, uint8_t u8Maximum);
    public:
    const Type eType;
    const uint8_t u8Minimum;
    const uint8_t u8Maximum;

    virtual void LockChevron();
    virtual void UnlockChevron();

    virtual void GoHome();
    virtual void Calibrate();

    virtual void LightUpChevron(Chevron eChevron, bool bIsActive);
    virtual void LightUpSymbol(uint8_t u8Symbol, bool bIsActive);   // Universe

    virtual void MoveToSymbol(uint8_t u8Symbol, Chevron eChevron);

    virtual void RunWormhole();
    virtual void ShutdownGate();

    virtual void UnlockGate();
    virtual void LockGate();
};


