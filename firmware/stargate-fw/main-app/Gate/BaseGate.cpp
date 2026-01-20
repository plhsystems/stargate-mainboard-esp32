#include "BaseGate.hpp"
#include "../HW/SGHW_HAL.hpp"

BaseGate::BaseGate(GateGalaxy gate_galaxy, const char* name) :
    gate_galaxy(gate_galaxy), name(name)
{

}
