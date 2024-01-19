#include "GateFactory.hpp"

BaseGate& GateFactory::Get(GateGalaxy eGateGalaxy)
{
    switch(eGateGalaxy)
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