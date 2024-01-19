#include "BaseGate.hpp"
#include "../HW/BoardHW.hpp"

BaseGate::BaseGate(GateGalaxy eGateGalaxy) :
    eGateGalaxy(eGateGalaxy)
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
    // TODO: Calculate ....
    BoardHW::MoveRelative(0);
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
