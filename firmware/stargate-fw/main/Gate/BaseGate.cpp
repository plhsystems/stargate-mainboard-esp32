#include "BaseGate.hpp"

BaseGate::BaseGate(Type eType, uint8_t u8Minimum, uint8_t u8Maximum) :
    eType(eType),
    u8Minimum(u8Minimum),
    u8Maximum(u8Maximum)
{

}

void BaseGate::LockChevron()
{

}

void BaseGate::UnlockChevron()
{

}

void BaseGate::GoHome()
{

}

void BaseGate::Calibrate()
{

}

void BaseGate::LightUpChevron(Chevron eChevron, bool bIsActive)
{

}

void BaseGate::LightUpSymbol(uint8_t u8Symbol, bool bIsActive)
{

}

void BaseGate::MoveToSymbol(uint8_t u8Symbol, Chevron eChevron)
{

}

void BaseGate::ShutdownGate()
{

}

void BaseGate::UnlockGate()
{

}

void BaseGate::LockGate()
{

}
