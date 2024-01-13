#include "GateControl.hpp"

GateControl::GateControl()
{
    // Constructor code here
}

void GateControl::StartTask()
{
}

void GateControl::QueueAction()
{

}

void GateControl::TaskRunning(GateControl* pc)
{
    // Get a stargate instance based on parameters
    BaseGate baseGate = GateFactory::Get(BaseGate::Type::Kristian);

    // Dialing
    while(true)
    {
        baseGate.UnlockGate(); //Only apply on universe

        baseGate.GoHome();

        BaseGate::Chevron currentChevron = BaseGate::Chevron::Chevron1;
        baseGate.MoveToSymbol(20, currentChevron);
        baseGate.LockChevron();
        // Pause
        baseGate.UnlockChevron();
        // Pause
        baseGate.LightUpSymbol(20, true); //Only apply on universe
        baseGate.LightUpChevron(currentChevron, true);

        baseGate.RunWormhole();
        baseGate.ShutdownGate();

        baseGate.GoHome();

        baseGate.LockGate(); //Only apply on universe
    }
}