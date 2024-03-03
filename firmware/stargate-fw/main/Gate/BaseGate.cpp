#include "BaseGate.hpp"
#include "../HW/SGHW_HAL.hpp"

BaseGate::BaseGate(GateGalaxy eGateGalaxy, const char* szName) :
    eGateGalaxy(eGateGalaxy), szName(szName)
{

}
