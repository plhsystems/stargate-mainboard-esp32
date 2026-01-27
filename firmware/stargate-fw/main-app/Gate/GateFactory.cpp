#include "GateFactory.hpp"

BaseGate& GateFactory::Get(GateGalaxy gate_galaxy)
{
    switch(gate_galaxy)
    {
        case GateGalaxy::Pegasus:
            return m_pegasusGate;
        case GateGalaxy::Universe:
            return m_universeGate;
        default:
        case GateGalaxy::MilkyWay:
            return m_milkyWayGate;
    }
}

UniverseGate& GateFactory::GetUniverseGate()
{
    return m_universeGate;
}